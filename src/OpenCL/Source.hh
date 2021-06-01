#ifndef OPENCL_SOURCE_HH
#define OPENCL_SOURCE_HH

#include <string>

#include <CL/cl2.hpp>

class Source
{
private:
    std::vector<std::string> str;
    cl::Program::Sources src;

public:
    Source(const char *url);
    Source(std::initializer_list<char *> urls);
    ~Source();

    operator cl::Program::Sources();
};

#endif