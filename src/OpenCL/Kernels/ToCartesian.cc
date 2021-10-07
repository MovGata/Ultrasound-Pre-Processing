#include "ToCartesian.hh"

namespace opencl
{

    ToCartesian::ToCartesian(const cl::Context &c, const cl::CommandQueue &q, const std::shared_ptr<opencl::Kernel> &ptr) : kernel(ptr), context(c), queue(q)
    {
        Filter::volume = std::make_shared<data::Volume>();
        Filter::input = std::bind(input, this, std::placeholders::_1);
        Filter::execute = std::bind(execute, this);
        Filter::getOptions = std::bind(getOptions, this);
    }

    void ToCartesian::input(const std::weak_ptr<data::Volume> &wv)
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

        volume->depth = indepth;
        volume->width = inwidth;
        volume->length = inlength;

        volume->buffer = cl::Buffer(context, CL_MEM_READ_WRITE, volume->length * volume->depth * volume->width * sizeof(cl_uint));
    }

    void ToCartesian::execute()
    {
        kernel->setArg(0, indepth);
        kernel->setArg(1, inlength);
        kernel->setArg(2, inwidth);
        kernel->setArg(3, inBuffer);
        kernel->setArg(4, volume->depth);
        kernel->setArg(5, volume->length);
        kernel->setArg(6, volume->width);
        kernel->setArg(7, volume->buffer);
        kernel->setArg(8, volume->ratio);
        kernel->setArg(9, volume->delta);

        kernel->global = cl::NDRange(volume->depth, volume->length, volume->width);
        kernel->execute(queue);
    }

    std::shared_ptr<gui::Tree> ToCartesian::getOptions()
    {
        std::shared_ptr<gui::Tree> options = gui::Tree::build("OPTIONS");
        return options;
    }
} // namespace opencl
