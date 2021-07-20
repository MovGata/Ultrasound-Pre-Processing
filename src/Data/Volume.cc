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


        // eventManager.addCallback(SDL_MOUSEWHEEL, events::scaleEvent<Volume>, *this);
        // eventManager.addCallback(SDL_MOUSEMOTION, events::rotateEvent<Volume>, *this);
    }

    Volume::Volume()
    {
        lastview = glm::mat4(1.0f);
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
        if (!modified)
        {
            return;
        }

        float maxEdge = static_cast<float>(std::max(std::max(depth, length), std::max(depth, width)));

        // glm::mat4 id(1.0f);
        
        glm::mat4 model(1.0f);

        // MODEL
        model = glm::scale(model, {maxEdge / static_cast<float>(depth), maxEdge / static_cast<float>(length), maxEdge / static_cast<float>(width)});
        model = glm::rotate(model, glm::radians(90.0f), {0.0f, 0.0f, -1.0f});

        glm::mat4 view(1.0f);

        // VIEW
        
        view = lastview;
        view = glm::translate(view, {translation.x, translation.y, translation.z});


        // id = glm::translate(id, {0.0f, 0.0f, 2.0f});
        view = model * view;


        GLfloat *modelView = glm::value_ptr(view);

        inv[0] = modelView[0];
        inv[1] = modelView[4];
        inv[2] = modelView[8];
        inv[3] = modelView[12];
        inv[4] = modelView[1];
        inv[5] = modelView[5];
        inv[6] = modelView[9];
        inv[7] = modelView[13];
        inv[8] = modelView[2];
        inv[9] = modelView[6];
        inv[10] = modelView[10];
        inv[11] = modelView[14];
    }

} // namespace data