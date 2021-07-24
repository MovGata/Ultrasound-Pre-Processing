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
    protected:
        Volume();
        std::vector<cl_uchar4> raw;

    public:
        glm::mat4 lastview;
        glm::vec3 translation = {0.0f, 0.0f, 5.0f};

        bool modified = false;

        cl_uchar max = 0;
        cl_uchar min = 0xFF;

        cl_uint depth;
        cl_uint length;
        cl_uint width;
        cl::Buffer buffer;
        cl_float ratio;
        cl_float delta;

        std::array<float, 12> inv = {0};

        Volume(const Volume &) = default;
        Volume(Volume &&) = default;
        Volume(unsigned int depth, unsigned int length, int unsigned width, const std::vector<uint8_t> &data);
        ~Volume();

        void sendToCl(const cl::Context &context);
        void update();
    };

}
#endif