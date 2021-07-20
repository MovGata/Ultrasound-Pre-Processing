#ifndef GUI_OPENCL_DEVICE
#define GUI_OPENCL_DEVICE

#include <map>
#include <memory>
#include <string>
#include <vector>

#include <gl/glew.h>
#include <gl/gl.h>
#include <gl/glext.h>

#include <CL/cl2.hpp>

#include "Program.hh"
#include "Concepts.hh"

namespace opencl
{

    class Device
    {
    private:
        cl::Platform platform;
        cl::Device device;
        cl::Buffer outBuffer;
        cl::Buffer invMVTransposed;

        unsigned int width = 0;
        unsigned int height = 0;

    public:
        cl_GLuint pixelBuffer;
        cl::CommandQueue cQueue;

        std::map<std::string, std::shared_ptr<Program>> programs;
        cl::Context context;
        cl_device_type type = CL_DEVICE_TYPE_CPU;

        Device(unsigned int width = 512, unsigned int height = 512);
        ~Device();

        template <concepts::VolumeType V>
        void render(V &v)
        {
            cl::NDRange global(width, height);
            
            if (v.buffer.template getInfo<CL_MEM_SIZE>() != v.length * v.depth * v.width * 4)
            {
                std::cerr << "invalid volume size." << '\n';
                std::terminate();
            }
            
            try
            {
                programs.at("raytracing")->at("render")->setArg(3, v.depth);
                programs.at("raytracing")->at("render")->setArg(4, v.length);
                programs.at("raytracing")->at("render")->setArg(5, v.width);
                programs.at("raytracing")->at("render")->setArg(6, v.buffer);
                
                cl_int err = 0;
                if (type == CL_DEVICE_TYPE_GPU)
                {
                    // Share GL buffer.
                    glFlush();
                    std::vector<cl::Memory> memories;
                    memories.push_back(outBuffer);
                    err |= cQueue.enqueueAcquireGLObjects(&memories);
                    err |= cQueue.enqueueWriteBuffer(invMVTransposed, CL_FALSE, 0, 12 * sizeof(float), v.inv.data());
                    err |= cQueue.enqueueNDRangeKernel(*programs.at("raytracing")->at("render"), cl::NullRange, global);
                    err |= cQueue.enqueueReleaseGLObjects(&memories);
                }
                else
                {
                    // Copy via host.
                    err |= cQueue.enqueueWriteBuffer(invMVTransposed, CL_FALSE, 0, 12 * sizeof(float), v.inv.data());
                    err |= cQueue.enqueueNDRangeKernel(*programs.at("raytracing")->at("render"), cl::NullRange, global);
                    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pixelBuffer);
                    GLubyte *p = static_cast<GLubyte *>(glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY));
                    auto bSize = outBuffer.getInfo<CL_MEM_SIZE>();
                    err |= cQueue.enqueueReadBuffer(outBuffer, CL_TRUE, 0, bSize, p);
                    glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
                }

                err = cQueue.finish();
                if (err != CL_SUCCESS)
                {
                    std::cerr << "clFinish Error: " << err << '\n';
                    std::terminate();
                }
            }
            catch (const cl::Error &e)
            {
                std::cerr << "Render, " << e.what() << " : " << e.err() << '\n';
                std::terminate();
            }
        }

        void createDisplay(unsigned int w, unsigned int h);

        void selectDevice();
    };

} // namespace opencl
#endif