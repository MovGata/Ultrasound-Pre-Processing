#include "MedianNoise.hh"

namespace opencl
{

    Median::Median(const Device &d) : kernel2D(d.programs.at("utility")->at("medianNoise2D")), kernel3D(d.programs.at("utility")->at("medianNoise3D")), context(d.context), queue(d.cQueue)
    {
        Filter::input = std::bind(input, this, std::placeholders::_1);
        Filter::execute = std::bind(execute, this);
        Filter::getOptions = std::bind(getOptions, this);
    }

    void Median::input(const std::weak_ptr<data::Volume> &wv)
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

    void Median::execute()
    {
        if (inwidth == 1)
        {
            kernel2D->setArg(0, indepth);
            kernel2D->setArg(1, inlength);
            kernel2D->setArg(2, inBuffer);
            kernel2D->setArg(3, volume->buffer);

            kernel2D->global = cl::NDRange(volume->depth, volume->length);
            kernel2D->execute(queue);

        }
        else
        {
            kernel3D->setArg(0, indepth);
            kernel3D->setArg(1, inlength);
            kernel3D->setArg(2, inwidth);
            kernel3D->setArg(3, inBuffer);
            kernel3D->setArg(4, volume->buffer);

            kernel3D->global = cl::NDRange(volume->depth, volume->length, volume->width);
            kernel3D->execute(queue);
        }

    }

    std::shared_ptr<gui::Tree> Median::getOptions()
    {
        std::shared_ptr<gui::Tree> options = gui::Tree::build("OPTIONS");
        return options;
    }
} // namespace opencl
