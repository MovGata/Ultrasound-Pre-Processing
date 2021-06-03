#ifndef GUI_OPENCL_DEVICE
#define GUI_OPENCL_DEVICE

#include <CL/cl2.hpp>

class Device
{
private:
    cl::Platform platform;
    cl::Device device;
    
    cl::Buffer   outBuffer;

    cl::Buffer invMVTransposed;

    cl::CommandQueue cQueue;
    cl::Program program;

    cl::Kernel renderK;


public:
    cl::Context context;
    cl_device_type type = CL_DEVICE_TYPE_CPU;

    Device();
    ~Device();
    

    void render(float *inv, cl_GLuint glPixelBuffer = 0);
    void createDisplay(unsigned int w, unsigned int h, cl_GLuint glPixelBuffer = 0);
    void prepareVolume(unsigned int d, unsigned int l, unsigned int w, const cl::Buffer &v);

    void selectDevice();


};

#endif