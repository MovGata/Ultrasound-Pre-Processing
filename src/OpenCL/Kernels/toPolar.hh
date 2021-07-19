#ifndef OPENCL_TOPOLAR_HH
#define OPENCL_TOPOLAR_HH

#include <memory>
#include <string>

#include "../Kernel.hh"
#include "../Concepts.hh"

#include "CL/cl2.hpp"

namespace opencl
{
    class ToPolar
    {
    private:
        std::shared_ptr<opencl::Kernel> kernel;
        cl_uint inlength;
        cl_uint inwidth;
        cl_uint indepth;
        cl::Buffer inBuffer;

    public:
        cl_float ratio;
        cl_float delta;

        cl_uint length;
        cl_uint depth;
        cl_uint width;
        cl::Buffer buffer;

        cl::Context context;
        cl::CommandQueue queue;

        const std::string in = "3D";
        const std::string out = "3D";

        ToPolar(const cl::Context &c, const cl::CommandQueue &q, const std::shared_ptr<opencl::Kernel> &ptr) : kernel(ptr), context(c), queue(q)
        {
        }
        
        ~ToPolar() = default;

        template<concepts::VolumeType V>
        void input(const V &v)
        {
            inlength = v.length;
            inwidth = v.width;
            indepth = v.depth;
            inBuffer = v.buffer;
            ratio = v.ratio;
            delta = v.delta;
            
            length = 512;
            width = 512;
            depth = 512;
            buffer = cl::Buffer(context, CL_MEM_READ_WRITE, length * depth * width * sizeof(cl_float));
        }

        void execute()
        {
            kernel->setArg(0, indepth);
            kernel->setArg(1, inlength);
            kernel->setArg(2, inwidth);
            kernel->setArg(3, inBuffer);
            kernel->setArg(4, depth);
            kernel->setArg(5, length);
            kernel->setArg(6, width);
            kernel->setArg(7, buffer);
            kernel->setArg(8, ratio);
            kernel->setArg(9, delta);
            

            kernel->global = cl::NDRange(depth, length, width);
            kernel->execute(queue);
        }
    };

} // namespace opencl

#endif