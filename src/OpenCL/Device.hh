#ifndef GUI_OPENCL_DEVICE
#define GUI_OPENCL_DEVICE

#include <chrono>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include <gl/glew.h>
#include <gl/gl.h>
#include <gl/glext.h>

#include <CL/cl2.hpp>

#include <SDL2/SDL_timer.h>

#include "Program.hh"
#include "Concepts.hh"

#include "../GUI/Renderer.hh"
#include "../GUI/Rectangle.hh"
#include "../GUI/Tree.hh"

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

        bool selected = false;

        Device(unsigned int width = 512, unsigned int height = 512);
        ~Device();

        void initialise();

        void render(gui::Renderer &renderer)
        {
            cl::NDRange global(width, height);
            
            if (renderer.tf->buffer.template getInfo<CL_MEM_SIZE>() != renderer.tf->length * renderer.tf->depth * renderer.tf->width * 4)
            {
                std::cerr << "invalid volume size." << '\n';
                std::terminate();
            }
            
            try
            {
                programs.at("raytracing")->at("render")->setArg(3, renderer.tf->depth);
                programs.at("raytracing")->at("render")->setArg(4, renderer.tf->length);
                programs.at("raytracing")->at("render")->setArg(5, renderer.tf->width);
                programs.at("raytracing")->at("render")->setArg(6, renderer.tf->buffer);
                
                cl_int err = 0;
                if (type == CL_DEVICE_TYPE_GPU)
                {
                    // Share GL buffer.
                    glFlush();
                    std::vector<cl::Memory> memories;
                    memories.push_back(outBuffer);
                    err |= cQueue.enqueueAcquireGLObjects(&memories);
                    err |= cQueue.enqueueWriteBuffer(invMVTransposed, CL_FALSE, 0, 12 * sizeof(float), renderer.inv.data());
                    err |= cQueue.enqueueNDRangeKernel(*programs.at("raytracing")->at("render"), cl::NullRange, global);
                    err |= cQueue.enqueueReleaseGLObjects(&memories);
                }
                else
                {
                    // Copy via host.
                    err |= cQueue.enqueueWriteBuffer(invMVTransposed, CL_FALSE, 0, 12 * sizeof(float), renderer.inv.data());
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

        std::shared_ptr<gui::Tree> buildDeviceTree(float x = 0.0f, float y = 0.0f);

        void selectDevice();
    };

} // namespace opencl
#endif