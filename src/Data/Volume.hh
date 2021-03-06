#ifndef DATA_VOLUME_HH
#define DATA_VOLUME_HH

#include <array>
#include <utility>
#include <vector>

#include <glm/glm.hpp>

#include <CL/cl2.hpp>
#include <SDL2/SDL.h>

#include "../events/EventManager.hh"

namespace data
{

    class Volume
    {
    public:
        Volume();
        std::vector<std::vector<cl_uchar4>> raw;


        cl_uchar max = 0;
        cl_uchar min = 0xFF;

        cl_uint depth;
        cl_uint length;
        cl_uint width;
        cl_uint frames;
        cl_uint cFrame;
        cl_uint rFrame;

        cl::Buffer buffer;
        cl_float ratio;
        cl_float delta;
        cl_float fRate;

        Volume(const Volume &) = default;
        Volume(Volume &&) = default;
        Volume(unsigned int depth, unsigned int length, int unsigned width, unsigned int frames, const std::vector<uint8_t> &data);
        ~Volume();

        std::vector<cl_uchar4> loadFromCl(const cl::CommandQueue &cQueue);
        void sendToCl(const cl::Context &context, unsigned int i);
        void update();
    };

}
#endif