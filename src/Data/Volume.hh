#ifndef DATA_VOLUME_HH
#define DATA_VOLUME_HH

#include <array>
#include <vector>

#include <CL/cl2.hpp>

class Volume
{
private:
    std::vector<cl_uchar4> raw;
    float rotation = 0.0f;
    unsigned int depth;
    unsigned int length;
    unsigned int width;

public:
    cl::Buffer buffer;
    std::array<float, 12> invMVTransposed;
    
    Volume(unsigned int depth, unsigned int length, int unsigned width, const std::vector<uint8_t> &data);
    ~Volume();

    void sendToCl(const cl::Context &context);
    void update();

};

#endif