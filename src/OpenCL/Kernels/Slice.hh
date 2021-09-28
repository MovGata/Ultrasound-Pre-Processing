#ifndef OPENCL_KERNELS_SLICE_HH
#define OPENCL_KERNELS_SLICE_HH

#include <array>
#include <memory>
#include <string>

#include <CL/cl2.hpp>

#include "../Filter.hh"
#include "../Kernel.hh"
#include "../Concepts.hh"
#include "../../Data/Volume.hh"
#include "../../GUI/Tree.hh"
#include "../../GUI/Slider.hh"

namespace opencl
{
    class Slice : public Filter
    {
    private:
        std::shared_ptr<opencl::Kernel> kernel;
        cl_uint inlength;
        cl_uint inwidth;
        cl_uint indepth;
        cl::Buffer inBuffer;

        cl::Buffer slices;

        std::array<std::shared_ptr<gui::Slider>, 3> slcSliders;
        std::array<float, 3> slc = {0.5f, 0.5f, 0.5f};

    public:
        cl::Context context;
        cl::CommandQueue queue;

        const std::string in = "3D";
        const std::string out = "3D";

        Slice(const cl::Context &c, const cl::CommandQueue &q, const std::shared_ptr<opencl::Kernel> &ptr) : kernel(ptr), context(c), queue(q)
        {
            Filter::volume = std::make_shared<data::Volume>();
            Filter::input = std::bind(input, this, std::placeholders::_1);
            Filter::execute = std::bind(execute, this);
            Filter::getOptions = std::bind(getOptions, this);

            slcSliders[0] = gui::Slider::build(0.0f, 0.0f, 0.0f, 10.0f);
            slcSliders[1] = gui::Slider::build(0.0f, 0.0f, 0.0f, 10.0f);
            slcSliders[2] = gui::Slider::build(0.0f, 0.0f, 0.0f, 10.0f);
        }

        ~Slice() = default;

        void input(const std::weak_ptr<data::Volume> &wv)
        {
            auto v = wv.lock();
            if (!v)
                return;

            volume->min = v->min;
            volume->max = v->max;
            inlength = v->length;
            inwidth = v->width;
            indepth = v->depth;
            inBuffer = v->buffer;
            volume->ratio = v->ratio;
            volume->delta = v->delta;
            volume->frames = v->frames;
            volume->fRate = v->fRate;

            volume->length = inlength;
            volume->width = inwidth;
            volume->depth = indepth;
            volume->buffer = cl::Buffer(context, CL_MEM_READ_WRITE, volume->length * volume->depth * volume->width * sizeof(cl_uint));

            slc[0] = slcSliders[0]->value;
            slc[1] = slcSliders[1]->value;
            slc[2] = slcSliders[2]->value;

            std::cout << slc[0] << ' ' << slc[1] << ' ' << slc[2] << std::endl;

            slices = cl::Buffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, 3 * sizeof(cl_float), slc.data());
        }

        void execute()
        {
            kernel->setArg(0, indepth);
            kernel->setArg(1, inlength);
            kernel->setArg(2, inwidth);
            kernel->setArg(3, inBuffer);
            kernel->setArg(4, volume->buffer);
            kernel->setArg(5, 1);
            kernel->setArg(6, 1);
            kernel->setArg(7, 1);
            kernel->setArg(8, slices);

            kernel->global = cl::NDRange(volume->depth, volume->length, volume->width);
            kernel->execute(queue);
        }
        std::shared_ptr<gui::Tree> getOptions();
    };

} // namespace opencl

#endif