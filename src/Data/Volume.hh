#ifndef DATA_VOLUME_HH
#define DATA_VOLUME_HH

#include <vector>

#include <CL/cl2.hpp>

class Volume
{
private:
    std::vector<cl_uchar4> raw;

public:
    cl::Buffer buffer;
    
    Volume(int depth, int length, int width, const std::vector<uint8_t> &data);
    ~Volume();

    void sendToCl(const cl::Context &context);

};

#endif