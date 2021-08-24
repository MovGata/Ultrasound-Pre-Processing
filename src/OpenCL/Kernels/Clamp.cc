#include "Clamp.hh"

namespace opencl
{

    Clamp::Clamp(const cl::Context &c, const cl::CommandQueue &q, const std::shared_ptr<opencl::Kernel> &ptr) : kernel(ptr), context(c), queue(q)
    {
        Filter::volume = std::make_shared<data::Volume>();
        Filter::input = std::bind(input, this, std::placeholders::_1);
        Filter::execute = std::bind(execute, this);
    }

    void Clamp::execute()
    {
        kernel->setArg(0, indepth);
        kernel->setArg(1, inlength);
        kernel->setArg(2, inwidth);
        kernel->setArg(3, inBuffer);
        kernel->setArg(4, volume->buffer);
        kernel->setArg(5, dl);
        kernel->setArg(6, du);
        kernel->setArg(7, ll);
        kernel->setArg(8, lu);
        kernel->setArg(9, wl);
        kernel->setArg(10, wu);

        kernel->global = cl::NDRange(volume->depth, volume->length, volume->width);
        kernel->execute(queue);
    }

} // namespace opencl
