#ifndef OPENCL_KERNELS_SLICE_HH
#define OPENCL_KERNELS_SLICE_HH

#include <array>
#include <memory>
#include <string>

#include <CL/cl2.hpp>

#include "../Kernel.hh"
#include "../Concepts.hh"
#include "../../Data/Volume.hh"


namespace opencl
{
    class Slice : public data::Volume
    {
    private:
        std::shared_ptr<opencl::Kernel> kernel;
        cl_uint inlength;
        cl_uint inwidth;
        cl_uint indepth;
        cl::Buffer inBuffer;

        cl::Buffer slices;

        std::array<float, 3> slc = {0.5f, 0.5f, 0.5f};

    public:
        cl::Context context;
        cl::CommandQueue queue;

        const std::string in = "3D";
        const std::string out = "3D";

        Slice(const cl::Context &c, const cl::CommandQueue &q, const std::shared_ptr<opencl::Kernel> &ptr) : kernel(ptr), context(c), queue(q)
        {
        }
        
        ~Slice() = default;

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
            slices = cl::Buffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, 3 * sizeof(cl_float), slc.data());
        }

        void execute()
        {
            kernel->setArg(0, indepth);
            kernel->setArg(1, inlength);
            kernel->setArg(2, inwidth);
            kernel->setArg(3, inBuffer);
            kernel->setArg(4, buffer);
            kernel->setArg(5, 1);
            kernel->setArg(6, 1);
            kernel->setArg(7, 1);
            kernel->setArg(8, slices);
            

            kernel->global = cl::NDRange(depth, length, width);
            kernel->execute(queue);

            modified = true;
        }
    };

} // namespace opencl

#endif