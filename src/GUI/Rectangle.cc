#include "Rectangle.hh"

#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glext.h>

#include <algorithm>
#include <iostream>

namespace gui
{

    Rectangle::Rectangle(float xp, float yp, float wp, float hp) : x(std::clamp(xp, -1.0f, 1.0f)),
                                                                   y(std::clamp(yp, -1.0f, 1.0f)),
                                                                   w(std::clamp(2.0f * wp, 0.0f, 1.0f - x)),
                                                                   h(std::clamp(2.0f * hp, 0.0f, 1.0f - y))
    {
    }

    Rectangle::~Rectangle()
    {
    }

    void Rectangle::allocTexture(unsigned int wp, unsigned int hp)
    {
        ww = wp;
        hh = hp;

        // Texture to render PBO into a quad
        glGenTextures(1, &texture);
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, wp, hp, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, 0);

        // PBO for OpenCL to render into
        glGenBuffers(1, &pixelBuffer);
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pixelBuffer);
        glBufferData(GL_PIXEL_UNPACK_BUFFER, wp * hp * sizeof(GLubyte) * 4, 0, GL_STREAM_DRAW);
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

        GLenum err;
        if ((err = glGetError()))
            std::cout << "err: " << err << std::endl;
    }

    void Rectangle::render() const
    {
        if (texture)
        {
            glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pixelBuffer);
            glBindTexture(GL_TEXTURE_2D, texture);
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, ww, hh, GL_RGBA, GL_UNSIGNED_BYTE, 0);

            // GLenum err;
            // if ((err = glGetError()))
            // {
            //     std::cout << "err: " << err << std::endl;
            // }

            glBegin(GL_QUADS);
            {
                glColor3f(0.4f, 0.4f, 0.4f);

                glTexCoord2f(0.0f, 0.0f);
                glVertex2f(x, y);

                glTexCoord2f(1.0f, 0.0f);
                glVertex2f(x + w, y);

                glTexCoord2f(1.0f, 1.0f);
                glVertex2f(x + w, y + h);

                glTexCoord2f(0.0f, 1.0f);
                glVertex2f(x, y + h);
            }
            glEnd();

            glBindTexture(GL_TEXTURE_2D, 0);
            glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
        }
        else
        {
            glBegin(GL_QUADS);
            {
                glColor3f(0.4f, 0.4f, 0.4f);
                glVertex2f(x, y);
                glVertex2f(x + w, y);
                glVertex2f(x + w, y + h);
                glVertex2f(x, y + h);
            }
            glEnd();
        }
    }

} // namespace gui