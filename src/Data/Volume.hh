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
    private:

        // std::pair<float, float> offset = {0.0f, 0.0f};

        glm::vec3 lastRotation;

    protected:
        Volume()=default;
        
        std::vector<cl_uchar4> raw;

    public:
        glm::mat4 lastview;
        glm::vec3 scale = {1.0f, 1.0f, 1.0f};
        glm::vec3 translation = {0.0f, 0.0f, 5.0f};
        glm::vec3 rotation = {0.0f, 0.0f, 0.0f};
        bool modified = false; // Render at least once

        cl_uint depth;
        cl_uint length;
        cl_uint width;
        cl::Buffer buffer;
        cl_float ratio;
        cl_float delta;

        std::array<float, 12> invMVTransposed = {0};

        Volume(const Volume &) = default;
        Volume(Volume &&) = default;
        Volume(unsigned int depth, unsigned int length, int unsigned width, const std::vector<uint8_t> &data);
        ~Volume();

        void sendToCl(const cl::Context &context);
        void update();
    };

}
#endif