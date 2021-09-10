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

    Volume::Volume(unsigned int d, unsigned int l, unsigned int w, unsigned int f, const std::vector<uint8_t> &data) : Volume()
    {
        depth = d;
        length = l;
        width = w;
        frames = f;

        raw.resize(frames);

        for (unsigned int v = 0; v < frames; ++v)
        {
            auto zyxv = v * length * depth * width;
            for (unsigned int z = 0; z < width; ++z)
            {
                auto zyx = z * length * depth;
                for (unsigned int y = 0; y < length; ++y)
                {
                    auto yx = y * depth;
                    for (unsigned int x = 0; x < depth; ++x)
                    {
                        cl_uchar bnw = data.at(x + yx + zyx + zyxv);
                        cl_uchar4 arr = {bnw, bnw, bnw, 0xFF};
                        raw[v].push_back(arr);
                    }
                }
            }
        }
    }

    Volume::Volume()
    {
    }

    Volume::~Volume()
    {
    }

    std::vector<cl_uchar4> Volume::loadFromCl(const cl::CommandQueue &cQueue)
    {
        auto bSize = buffer.getInfo<CL_MEM_SIZE>();
        std::vector<cl_uchar4> bVec(bSize);
        cQueue.enqueueReadBuffer(buffer, CL_TRUE, 0, bSize, bVec.data()); // Rework into non-blocking
        cQueue.finish();
        return bVec;
    }

    void Volume::sendToCl(const cl::Context &context, unsigned int i)
    {
        buffer = cl::Buffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(raw[i][0]) * raw[i].size(), raw[i].data());
    }

    void Volume::update()
    {
    }

} // namespace data