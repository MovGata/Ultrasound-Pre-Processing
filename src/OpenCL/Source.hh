#ifndef OPENCL_SOURCE_HH
#define OPENCL_SOURCE_HH

#include <string>

#include <CL/cl2.hpp>

class Source
{
private:
    cl::Program::Sources src;

public:
    std::string name;

    Source(const std::string &url);
    // Source(std::initializer_list<char *> urls);
    ~Source();

    operator cl::Program::Sources();
};

#endif