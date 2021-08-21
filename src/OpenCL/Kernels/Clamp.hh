#ifndef OPENCL_KERNELS_CLAMP_HH
#define OPENCL_KERNELS_CLAMP_HH

#include <array>
#include <memory>
#include <string>

#include <CL/cl2.hpp>

#include "../Kernel.hh"
#include "../Concepts.hh"
#include "../../Data/Volume.hh"

namespace opencl
{
    class Clamp : public data::Volume
    {
    private:
        std::shared_ptr<opencl::Kernel> kernel;
        cl_uint inlength;
        cl_uint inwidth;
        cl_uint indepth;
        cl::Buffer inBuffer;

        float dl = 0.25f, du = 0.75f, ll = 0.25f, lu = 0.75f, wl = 0.5f, wu = 1.0f;

    public:
        cl::Context context;
        cl::CommandQueue queue;

        const std::string in = "3D";
        const std::string out = "3D";

        Clamp(const cl::Context &c, const cl::CommandQueue &q, const std::shared_ptr<opencl::Kernel> &ptr);
        ~Clamp() = default;

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

            length = inlength;
            width = inwidth;
            depth = indepth;
            buffer = cl::Buffer(context, CL_MEM_READ_WRITE, length * depth * width * sizeof(cl_uint));
        }

        void execute();
    };

} // namespace opencl

#endif