#include "Rectangle.hh"

#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glext.h>

#include <algorithm>
#include <cmath>
#include <iostream>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

namespace gui
{
    const Uint32 Rectangle::dropEvent = SDL_RegisterEvents(1);

    Rectangle::Rectangle(float xp, float yp, float wp, float hp) : //text(nullptr, SDL_FreeSurface),
                                                                   x(std::clamp(xp, -1.0f, 1.0f)),
                                                                   y(std::clamp(yp, -1.0f, 1.0f)),
                                                                   w(std::clamp(2.0f * wp, 0.0f, 1.0f - x)),
                                                                   h(std::clamp(2.0f * hp, 0.0f, 1.0f - y))
    {
        addCallback(SDL_MOUSEWHEEL, std::bind(Rectangle::zoom, this, std::placeholders::_1));
        addCallback(SDL_MOUSEBUTTONDOWN, std::bind(Rectangle::mouseLeftDown, this, std::placeholders::_1));
        addCallback(SDL_MOUSEBUTTONUP, std::bind(Rectangle::mouseLeftUp, this, std::placeholders::_1));
        addCallback(SDL_MOUSEMOTION, std::bind(Rectangle::mouseMotion, this, std::placeholders::_1));
        addCallback(dropEvent, std::bind(Rectangle::userDrop, this, std::placeholders::_1));
    }

    Rectangle::~Rectangle()
    {
    }

    Volume &Rectangle::allocVolume(unsigned int depth, unsigned int length, int unsigned width, const std::vector<uint8_t> &data)
    {
        return volume.emplace(depth, length, width, data);
    }

    void Rectangle::allocTexture(unsigned int wp, unsigned int hp)
    {
        ww = wp;
        hh = hp;

        // Texture to render PBO into a quad
        glGenTextures(1, &texture);
        glEnable(GL_TEXTURE_2D);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
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
                glColor4ub(colour.r, colour.g, colour.b, colour.a);

                glTexCoord2f(0.0f, 1.0f);
                glVertex2f(x + offX, y + offY);

                glTexCoord2f(1.0f, 1.0f);
                glVertex2f(x + offX + w, y + offY);

                glTexCoord2f(1.0f, 0.0f);
                glVertex2f(x + offX + w, y + offY + h);

                glTexCoord2f(0.0f, 0.0f);
                glVertex2f(x + offX, y + offY + h);
            }
            glEnd();

            glBindTexture(GL_TEXTURE_2D, 0);
            glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
        }
        else
        {
            glBegin(GL_QUADS);
            {
                glColor4ub(colour.r, colour.g, colour.b, colour.a);
                glVertex2f(x + offX, y + offY);
                glVertex2f(x + offX + w, y + offY);
                glVertex2f(x + offX + w, y + offY + h);
                glVertex2f(x + offX, y + offY + h);
            }
            glEnd();
        }
    }

    void Rectangle::addText(TTF_Font &font, const std::string &str)
    {
        if (texture == 0)
        {
            return;
        }
        SDL_Surface *s = TTF_RenderText_Blended(&font, str.c_str(), SDL_Colour{0xFF, 0xFF, 0xFF, 0xFF});
        if (!s)
        {
            std::cout << "Failed to create text" << std::endl;
        }
        SDL_Surface *textT = SDL_ConvertSurfaceFormat(s, SDL_PixelFormatEnum::SDL_PIXELFORMAT_RGBA8888, 0);
        SDL_FreeSurface(s);
        for (int i = 0; i < textT->w * textT->h; ++i)
        {
            uint8_t t = static_cast<uint8_t *>(textT->pixels)[i * 4];
            static_cast<uint8_t *>(textT->pixels)[i * 4] = static_cast<uint8_t *>(textT->pixels)[i * 4 + 3];
            static_cast<uint8_t *>(textT->pixels)[i * 4 + 3] = t;
        }
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pixelBuffer);
        if (textT->w * textT->h < ww * hh)
        {
            glBufferSubData(GL_PIXEL_UNPACK_BUFFER, 0, (textT->w * textT->h) * sizeof(GLubyte) * 4, textT->pixels);

            ww = textT->w;
            hh = textT->h;
        }
        else
        {
            glBufferSubData(GL_PIXEL_UNPACK_BUFFER, 0, (ww * hh) * sizeof(GLubyte) * 4, textT->pixels);
        }
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

        SDL_FreeSurface(textT);
        
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glBindTexture(GL_TEXTURE_2D, 0);
        text = str;
    }

    void Rectangle::setBG(SDL_Colour c)
    {
        colour = c;
    }

    void Rectangle::addCallback(Uint32 e, std::function<void(const SDL_Event &)> fun)
    {
        events.insert_or_assign(e, fun);
    }

    void Rectangle::process(const SDL_Event &e)
    {
        auto itr = events.find(e.type);
        if (itr != events.end())
        {
            itr->second(e);
        }
    }

    void Rectangle::zoom(const SDL_Event &e)
    {
        if (volume.has_value())
        {
            volume->zoom(static_cast<float>(e.wheel.y));
        }
    }

    void Rectangle::mouseMotion(const SDL_Event &e)
    {
        if (mouseDown)
        {
            // std::cout << "Rotate: (" << e.motion.xrel << ", " << e.motion.yrel << '\n';
            if (volume.has_value())
            {
                volume->rotate(static_cast<float>(e.motion.xrel), static_cast<float>(e.motion.yrel));
            }
            else if (draggable)
            {
                int width, height;
                SDL_GetWindowSize(SDL_GetWindowFromID(e.motion.windowID), &width, &height);
                width = height = std::min(width, height);
                offX += static_cast<float>(2 * e.motion.xrel) / static_cast<float>(width);
                offY += static_cast<float>(2 * e.motion.yrel) / static_cast<float>(-height);
                // std::cout << offX << ',' << offY << std::endl;
            }
        }
    }

    void Rectangle::mouseLeftUp([[maybe_unused]] const SDL_Event &e)
    {
        mouseDown = false;

        offX = 0;
        offY = 0;
    }

    void Rectangle::mouseLeftDown([[maybe_unused]] const SDL_Event &e)
    {
        mouseDown = true;

        if (draggable)
        {
            SDL_Event ev;
            ev.type = dropEvent;
            ev.user.code = 0;
            ev.user.data1 = this;
            // std::cout << x << ' ' << y << " " << offX << " " << offY << std::endl; 
            SDL_PushEvent(&ev);
        }
    }

    void Rectangle::userDrop([[maybe_unused]] const SDL_Event &e)
    {
        // std::cout << "Dropped: " << x << y << ' ' << *static_cast<float *>(e.user.data1) << ' ' << *static_cast<float *>(e.user.data2) << std::endl;
        std::cout << x << ", " << y << " Dropped: " << static_cast<Rectangle *>(e.user.data1)->text << std::endl;
    }

} // namespace gui