#include "Kernel.hh"

#include <CL/cl2.hpp>

namespace opencl
{

    Kernel::Kernel(cl::Kernel k) : kernel(k)
    {
    }

    Kernel::~Kernel()
    {
    }

    Kernel::operator cl::Kernel()
    {
        return kernel;
    }

    std::string Kernel::getArg(unsigned int pos)
    {
        return kernel.getArgInfo<CL_KERNEL_ARG_NAME>(pos);
    }

    bool Kernel::isInput(unsigned int pos)
    {
        auto addr = kernel.getArgInfo<CL_KERNEL_ARG_ADDRESS_QUALIFIER>(pos);
        auto acce = kernel.getArgInfo<CL_KERNEL_ARG_ACCESS_QUALIFIER>(pos);

        return (addr == CL_KERNEL_ARG_ADDRESS_CONSTANT || acce != CL_KERNEL_ARG_ACCESS_WRITE_ONLY);
    }

    bool Kernel::isOutput(unsigned int pos)
    {
        auto addr = kernel.getArgInfo<CL_KERNEL_ARG_ADDRESS_QUALIFIER>(pos);
        auto acce = kernel.getArgInfo<CL_KERNEL_ARG_ACCESS_QUALIFIER>(pos);

        return (addr != CL_KERNEL_ARG_ADDRESS_CONSTANT && (acce == CL_KERNEL_ARG_ACCESS_WRITE_ONLY || acce == CL_KERNEL_ARG_ACCESS_READ_WRITE));
    }

    cl_uint Kernel::numArgs()
    {
        return kernel.getInfo<CL_KERNEL_NUM_ARGS>();
    }

    void Kernel::execute(cl::CommandQueue &cQueue)
    {
        try
        {
            cl_int err = 0;

            err |= cQueue.enqueueNDRangeKernel(kernel, cl::NullRange, global);

            if (err != CL_SUCCESS)
            {
                std::cerr << err << '\n';
                std::terminate();
            }
        }
        catch (const cl::Error &e)
        {
            std::cerr << "Finish, " << e.what() << " : " << e.err() << '\n';
            std::terminate();
        }
    }

} // namespace opencl
