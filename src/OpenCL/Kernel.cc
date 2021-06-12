#include "Kernel.hh"

#include <CL/cl2.hpp>

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