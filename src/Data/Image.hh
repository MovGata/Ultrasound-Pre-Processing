#ifndef DATA_IMAGE_HH
#define DATA_IMAGE_HH

#include <CL/cl2.hpp>

template <typename T>
class Image
{
private:
    cl::Image2D image;

public:
    std::size_t w, h;

    Image(std::size_t width, std::size_t height, T type);
    ~Image();
};

#endif