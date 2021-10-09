#include "Shrink.hh"

namespace opencl
{

    Shrink::Shrink(const cl::Context &c, const cl::CommandQueue &q, const std::shared_ptr<opencl::Kernel> &ptr) : kernel(ptr), context(c), queue(q)
    {
        Filter::input = std::bind(input, this, std::placeholders::_1);
        Filter::execute = std::bind(execute, this);
        Filter::getOptions = std::bind(getOptions, this);
    }

    void Shrink::input(const std::weak_ptr<data::Volume> &wv)
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
        volume->rFrame = v->rFrame;
        volume->cFrame = v->cFrame;

        volume->length = inlength;
        volume->width = inwidth;
        volume->depth = indepth;
        volume->buffer = cl::Buffer(context, CL_MEM_READ_WRITE, volume->length * volume->depth * volume->width * sizeof(cl_uint));
    }

    void Shrink::execute()
    {
        kernel->setArg(0, indepth);
        kernel->setArg(1, inlength);
        kernel->setArg(2, inwidth);
        kernel->setArg(3, inBuffer);
        kernel->setArg(4, volume->buffer);

        kernel->global = cl::NDRange(volume->depth, volume->length, volume->width);
        kernel->execute(queue);
    }

    std::shared_ptr<gui::Tree> Shrink::getOptions()
    {
        std::shared_ptr<gui::Tree> options = gui::Tree::build("OPTIONS");
        return options;
    }
} // namespace opencl
