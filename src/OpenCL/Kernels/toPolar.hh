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

        cl_float inRatio;
        cl_float angleDelta;

        cl_uint outlength;
        cl_uint outdepth;
        cl_uint outwidth;
        cl::Buffer outBuffer;

    public:
        const std::string in = "3D";
        const std::string out = "2D";

        ToPolar(const std::shared_ptr<opencl::Kernel> &ptr) : kernel(ptr)
        {
        }
        
        ~ToPolar() = default;

        template<concepts::VolumeType V>
        void input(cl::Context &context, const V &v)
        {
            inlength = v.length;
            inwidth = v.width;
            indepth = v.depth;
            inBuffer = v.Buffer;
            inRatio = v.ratio;
            angleDelta = v.delta;
            
            outlength = 512;
            outwidth = 512;
            outdepth = 512;
            outBuffer = cl::Buffer(context, CL_MEM_READ_WRITE, outlength * outdepth * outwidth * sizeof(cl_float));
        }

        void execute(cl::CommandQueue &cQueue)
        {
            kernel->setArg(0, indepth);
            kernel->setArg(1, inlength);
            kernel->setArg(2, inwidth);
            kernel->setArg(3, inBuffer);
            kernel->setArg(4, outdepth);
            kernel->setArg(5, outlength);
            kernel->setArg(6, outwidth);
            kernel->setArg(7, outBuffer);
            kernel->setArg(8, inRatio);
            kernel->setArg(9, angleDelta);
            

            kernel->global = cl::NDRange(outdepth, outlength, outwidth);
            kernel->execute(cQueue);
        }

        void cleanup([[maybe_unused]] cl::CommandQueue &cQueue)
        {

        }
    };

} // namespace opencl

#endif