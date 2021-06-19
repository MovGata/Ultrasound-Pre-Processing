#ifndef OPENCL_PROGRAM_HH
#define OPENCL_PROGRAM_HH

#include <string>
#include <map>

#include <CL/cl2.hpp>

#include "Kernel.hh"
#include "Source.hh"

class Program
{
private:
    cl::Program program;
    std::map<std::string, Kernel> kernels;

public:
    std::string name;
    
    Program(cl::Context context, Source src);
    ~Program();

    Kernel &at(std::string str);
    // cl::Program *operator->();
};

#endif