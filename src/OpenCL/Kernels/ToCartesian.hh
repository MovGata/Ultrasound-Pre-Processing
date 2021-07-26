#ifndef OPENCL_TOCARTESIAN_HH
#define OPENCL_TOCARTESIAN_HH

#include <cmath>
#include <memory>
#include <string>

#include <CL/cl2.hpp>

#include "../Kernel.hh"
#include "../Concepts.hh"
#include "../../Data/Volume.hh"

namespace opencl
{
    class ToCartesian : public data::Volume
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

        ToCartesian(const cl::Context &c, const cl::CommandQueue &q, const std::shared_ptr<opencl::Kernel> &ptr) : kernel(ptr), context(c), queue(q)
        {
        }

        ~ToCartesian() = default;

        template <concepts::VolumeType V>
        void input(const V &v)
        {
            min = v.min;
            max = v.max;
            inlength = v.length;
            inwidth = v.width;
            indepth = v.depth;
            inBuffer = v.buffer;
            ratio = v.ratio;
            delta = v.delta;

            depth = static_cast<cl_uint>(static_cast<float>(v.depth) + static_cast<float>(v.depth) * v.ratio + 1.0f);
            length = static_cast<cl_uint>(2.0f * std::tan(v.delta / 2.0f) * static_cast<float>(depth) + 1.0f);
            width = inwidth > 1 ? static_cast<cl_uint>(2.0f * std::tan(v.delta / 2.0f) * static_cast<float>(depth) + 1.0f) : inwidth;

            std::cout << indepth << '=' << depth << '\n' << inlength << '=' << length << '\n' << inwidth << '=' << width << std::endl;

            buffer = cl::Buffer(context, CL_MEM_READ_WRITE, length * depth * width * sizeof(cl_uint));
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

            modified = true;
        }
    };

} // namespace opencl

#endif