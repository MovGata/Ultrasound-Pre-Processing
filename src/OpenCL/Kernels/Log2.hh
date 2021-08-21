#ifndef OPENCL_KERNELS_LOG2_HH
#define OPENCL_KERNELS_LOG2_HH

#include <memory>
#include <string>

#include <CL/cl2.hpp>

#include "../Kernel.hh"
#include "../Concepts.hh"
#include "../../Data/Volume.hh"


namespace opencl
{
    class Log2 : public data::Volume
    {
    private:
        std::shared_ptr<opencl::Kernel> kernel;
        cl_uint inlength;
        cl_uint inwidth;
        cl_uint indepth;
        cl::Buffer inBuffer;

    public:
        cl::Context context;
        cl::CommandQueue queue;

        const std::string in = "3D";
        const std::string out = "3D";

        Log2(const cl::Context &c, const cl::CommandQueue &q, const std::shared_ptr<opencl::Kernel> &ptr) : kernel(ptr), context(c), queue(q)
        {
        }
        
        ~Log2() = default;

        template<concepts::VolumeType V>
        void input(const std::weak_ptr<V> &wv)
        {
            auto v = wv.lock();
            if (!v)
                return;
                
            min = v->min;
            max = v->max;
            inlength = v->length;
            inwidth = v->width;
            indepth = v->depth;
            inBuffer = v->buffer;
            ratio = v->ratio;
            delta = v->delta;
            
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
            
            kernel->global = cl::NDRange(depth, length, width);
            kernel->execute(queue);

            min = static_cast<cl_uchar>(std::log2(min));
            max = static_cast<cl_uchar>(std::log2(max));

            modified = true;
        }
    };

} // namespace opencl

#endif