#ifndef OPENCL_PROGRAM_HH
#define OPENCL_PROGRAM_HH

#include <string>
#include <map>

#include <CL/cl2.hpp>

#include "Kernel.hh"
#include "Source.hh"

namespace opencl
{

class Program
{
private:
    cl::Program program;

public:
    std::map<std::string, Kernel> kernels;
    std::string name;
    
    Program(cl::Context context, Source src);
    ~Program();

    Kernel &at(std::string str);
    // cl::Program *operator->();
};
} // namespace opencl

#endif