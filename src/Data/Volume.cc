#include "Volume.hh"

#include <gl/glew.h>
#include <gl/gl.h>
#include <gl/glext.h>

Volume::Volume(unsigned int d, unsigned int l, unsigned int w, const std::vector<uint8_t> &data) : depth(d), length(l), width(w)
{
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
    float maxEdge = static_cast<float>(std::max(std::max(depth, length), std::max(depth, width)));

    // rotation = rotation + 1.0f;
    GLfloat modelView[16];
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glScalef(maxEdge / static_cast<float>(depth) * 0.1f, maxEdge / static_cast<float>(length) * 0.1f, maxEdge / static_cast<float>(width) * 0.1f);
    glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
    glTranslatef(0.0f, 0.0f, 25.0f);
    glGetFloatv(GL_MODELVIEW_MATRIX, modelView);
    glPopMatrix();

    invMVTransposed[0] = modelView[0];
    invMVTransposed[1] = modelView[4];
    invMVTransposed[2] = modelView[8];
    invMVTransposed[3] = modelView[12];
    invMVTransposed[4] = modelView[1];
    invMVTransposed[5] = modelView[5];
    invMVTransposed[6] = modelView[9];
    invMVTransposed[7] = modelView[13];
    invMVTransposed[8] = modelView[2];
    invMVTransposed[9] = modelView[6];
    invMVTransposed[10] = modelView[10];
    invMVTransposed[11] = modelView[14];
}
