#include "Volume.hh"

#include <numbers>

#include <gl/glew.h>
#include <gl/gl.h>
#include <gl/glext.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "../Events/GUI.hh"

namespace data
{

    Volume::Volume(unsigned int d, unsigned int l, unsigned int w, const std::vector<uint8_t> &data) : Volume()
    {
        depth = d;
        length = l;
        width = w;
        
        raw.reserve(width * depth * length);
        for (unsigned int z = 0; z < width; ++z)
        {
            auto zyx = z * length * depth;
            for (unsigned int y = 0; y < length; ++y)
            {
                auto yx = y * depth;
                for (unsigned int x = 0; x < depth; ++x)
                {
                    cl_uchar bnw = data.at(x + yx + zyx);
                    cl_uchar4 arr = {bnw, bnw, bnw, 0xFF};
                    raw.push_back(arr);
                }
            }
        }


        // eventManager->addCallback(SDL_MOUSEWHEEL, events::scaleEvent<Volume>, *this);
        // eventManager->addCallback(SDL_MOUSEMOTION, events::rotateEvent<Volume>, *this);
    }

    Volume::Volume()
    {
    }

    Volume::~Volume()
    {
    }

    void Volume::sendToCl(const cl::Context &context)
    {
        buffer = cl::Buffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(raw[0]) * raw.size(), raw.data());
    }

    void Volume::update()
    {
    }

} // namespace data