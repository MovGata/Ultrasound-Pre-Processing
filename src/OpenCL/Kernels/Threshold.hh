#ifndef OPENCL_KERNELS_THRESHOLD_HH
#define OPENCL_KERNELS_THRESHOLD_HH

#include <memory>
#include <string>

#include <CL/cl2.hpp>

#include "../Kernel.hh"
#include "../Concepts.hh"
#include "../../Data/Volume.hh"


namespace opencl
{
    class Threshold : public data::Volume
    {
    private:
        std::shared_ptr<opencl::Kernel> kernel;
        cl_uint inlength;
        cl_uint inwidth;
        cl_uint indepth;
        cl::Buffer inBuffer;

        cl_uchar threshold = 0xFF/8;

    public:
        cl::Context context;
        cl::CommandQueue queue;

        const std::string in = "3D";
        const std::string out = "3D";

        Threshold(const cl::Context &c, const cl::CommandQueue &q, const std::shared_ptr<opencl::Kernel> &ptr) : kernel(ptr), context(c), queue(q)
        {
        }
        
        ~Threshold() = default;

        template<concepts::VolumeType V>
        void input(const V &v)
        {
            inlength = v.length;
            inwidth = v.width;
            indepth = v.depth;
            inBuffer = v.buffer;
            ratio = v.ratio;
            delta = v.delta;
            
            length = inlength;
            width = inwidth;
            depth = indepth;
            buffer = cl::Buffer(context, CL_MEM_READ_WRITE, length * depth * width * sizeof(cl_uint));
        }

        void execute()
        {
            kernel->setArg(0, indepth);
            kernel->setArg(1, inlength);
            kernel->setArg(2, inwidth);
            kernel->setArg(3, inBuffer);
            kernel->setArg(4, buffer);
            kernel->setArg(5, threshold);
            

            kernel->global = cl::NDRange(depth, length, width);
            kernel->execute(queue);

            modified = true;
        }
    };

} // namespace opencl

#endif