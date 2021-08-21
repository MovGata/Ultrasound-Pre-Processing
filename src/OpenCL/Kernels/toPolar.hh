#ifndef OPENCL_TOPOLAR_HH
#define OPENCL_TOPOLAR_HH

#include <cmath>
#include <memory>
#include <string>

#include <CL/cl2.hpp>

#include "../Kernel.hh"
#include "../Concepts.hh"
#include "../../Data/Volume.hh"

namespace opencl
{
    class ToPolar : public data::Volume
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

        ToPolar(const cl::Context &c, const cl::CommandQueue &q, const std::shared_ptr<opencl::Kernel> &ptr) : kernel(ptr), context(c), queue(q)
        {
        }

        ~ToPolar() = default;

        template <concepts::VolumeType V>
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

            depth = static_cast<cl_uint>(static_cast<float>(v->depth) + (static_cast<float>(v->depth) * v->ratio) + 1.0f);
            length = static_cast<cl_uint>(2.0f * std::tan(v->delta / 2.0f) * static_cast<float>(depth) + 1.0f);
            width = inwidth > 1 ? static_cast<cl_uint>(2.0f * std::tan(v->delta / 2.0f) * static_cast<float>(depth) + 1.0f) : inwidth;
            depth -= static_cast<cl_uint>(static_cast<float>(v->depth) * v->ratio * std::cos(std::asin(std::sqrt(std::pow(std::sin(v->delta / 2.0f) * (static_cast<float>(v->depth) * v->ratio), 2) + std::pow(std::sin(v->delta / 2.0f) * (static_cast<float>(v->depth) * v->ratio), 2)) / (static_cast<float>(v->depth) * v->ratio))));

            std::cout << indepth << '=' << depth << '\n'
                      << inlength << '=' << length << '\n'
                      << inwidth << '=' << width << std::endl;

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