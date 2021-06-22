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
    const Uint32 Rectangle::dropEventData = SDL_RegisterEvents(2);
    const Uint32 Rectangle::volumeEventData = Rectangle::dropEventData + 1;

    Rectangle::Rectangle(float xp, float yp, float wp, float hp) : //text(nullptr, SDL_FreeSurface),
                                                                   x(std::clamp(xp, -1.0f, 1.0f)),
                                                                   y(std::clamp(yp, -1.0f, 1.0f)),
                                                                   w(std::clamp(2.0f * wp, 0.0f, 1.0f - x)),
                                                                   h(std::clamp(2.0f * hp, 0.0f, 1.0f - y))
    {
    }

    Rectangle::~Rectangle()
    {
    }

    Volume &Rectangle::allocVolume(unsigned int depth, unsigned int length, int unsigned width, const std::vector<uint8_t> &data)
    {
        addCallback(SDL_MOUSEWHEEL, std::bind(volumeBypass, this, std::placeholders::_1));
        addCallback(SDL_MOUSEBUTTONDOWN, std::bind(volumeStartEvent, this, std::placeholders::_1));
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

    void Rectangle::render(float xp, float yp, float wp, float hp) const
    {
        float xlerp = std::lerp(xp, xp + 2.0f * wp, (x + 1.0f) / 2.0f);
        float ylerp = std::lerp(yp, yp + 2.0f * hp, (y + 1.0f) / 2.0f);
        float wlerp = xlerp + std::lerp(xp, xp + 2.0f * wp, w / 2.0f) + 1.0f;
        float hlerp = ylerp + std::lerp(yp, yp + 2.0f * hp, h / 2.0f) + 1.0f;

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
            // float wlerp = std::lerp((xp + 1.0f) * 2.0f, xp + wp, w / 2.0f);
            // float hlerp = std::lerp(yp, xp + wp, h);

            glBegin(GL_QUADS);
            {
                glColor4ub(colour.r, colour.g, colour.b, colour.a);

                glTexCoord2f(0.0f, 1.0f);
                glVertex2f(xlerp + offX, ylerp + offY);

                glTexCoord2f(1.0f, 1.0f);
                glVertex2f(wlerp + offX, ylerp + offY);

                glTexCoord2f(1.0f, 0.0f);
                glVertex2f(wlerp + offX, hlerp + offY);

                glTexCoord2f(0.0f, 0.0f);
                glVertex2f(xlerp + offX, hlerp + offY);
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
                glVertex2f(xlerp + offX, ylerp + offY);
                glVertex2f(wlerp + offX, ylerp + offY);
                glVertex2f(wlerp + offX, hlerp + offY);
                glVertex2f(xlerp + offX, hlerp + offY);
            }
            glEnd();
        }

        // for (const auto &r : subRectangles)
        // {
        //     r.render();
        // }
    }

    void Rectangle::addText(TTF_Font *f, const std::string &str)
    {
        if (texture == 0)
        {
            return;
        }
        font = f;
        SDL_Surface *s = TTF_RenderText_Blended(font, str.c_str(), SDL_Colour{0xFF, 0xFF, 0xFF, 0xFF});
        if (!s)
        {
            std::cerr << "Failed to create text: " << TTF_GetError() << '\n';
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

        addCallback(SDL_MOUSEBUTTONDOWN, std::bind(dragStartEvent, this, std::placeholders::_1));
    }

    void Rectangle::setBG(SDL_Colour c)
    {
        colour = c;
    }

    // DRAG AND DROP EVENTS

    void Rectangle::dragEvent(const SDL_Event &e)
    {
        int width, height;
        SDL_GetWindowSize(SDL_GetWindowFromID(e.motion.windowID), &width, &height);
        width = height = std::min(width, height);
        offX += static_cast<float>(2 * e.motion.xrel) / static_cast<float>(width);
        offY += static_cast<float>(2 * e.motion.yrel) / static_cast<float>(-height);
    }

    void Rectangle::dragStopEvent([[maybe_unused]] const SDL_Event &e)
    {
        offX = 0;
        offY = 0;

        clearCallback(SDL_MOUSEBUTTONUP);
        clearCallback(SDL_MOUSEMOTION);
    }

    void Rectangle::dragStartEvent([[maybe_unused]] const SDL_Event &e)
    {
        SDL_Event ev;
        ev.type = dropEventData;
        ev.user.windowID = e.button.windowID;
        ev.user.code = 0;
        ev.user.data1 = this;
        SDL_PushEvent(&ev);

        addCallback(SDL_MOUSEBUTTONUP, std::bind(Rectangle::dragStopEvent, this, std::placeholders::_1));
        addCallback(SDL_MOUSEMOTION, std::bind(Rectangle::dragEvent, this, std::placeholders::_1));
    }

    void Rectangle::dropEvent([[maybe_unused]] const SDL_Event &e)
    {
        // std::cout << "Dropped: " << x << y << ' ' << *static_cast<float *>(e.user.data1) << ' ' << *static_cast<float *>(e.user.data2) << std::endl;
        Rectangle *rp = static_cast<Rectangle *>(e.user.data1);
        // int mx, my;
        // SDL_GetMouseState(&mx, &my);
        // int width, height, wd, hd;
        // SDL_GetWindowSize(SDL_GetWindowFromID(e.motion.windowID), &width, &height);
        // wd = hd = std::min(width, height);
        // my = height - my;
        // Rectangle &r = subRectangles.emplace_back(2.0f * static_cast<float>(mx - ((width - wd) / 2)) / static_cast<float>(wd) - 1.0f,
        //                                           2.0f * static_cast<float>(my - ((height - hd) / 2)) / static_cast<float>(hd) - 1.0f, rp->w / 2.0f, rp->h / 2.0f);
        Rectangle &r = subRectangles.emplace_back(rp->x + rp->offX, rp->y + rp->offY, rp->w, rp->h);
        r.setBG({0xFF, 0xFF, 0xFF, 0xFF});
        // r.allocTexture(rp->ww, rp->hh);
        // r.text = rp->text;
        // r.addText(rp->font, r.text);
        r.texture = rp->texture;
        r.pixelBuffer = rp->pixelBuffer;
        r.ww = rp->ww;
        r.hh = rp->hh;
        std::cout << subRectangles.size() << std::endl;
    }

    // VOLUME EVENTS

    void Rectangle::volumeBypass(const SDL_Event &e)
    {
        volume->process(e);
    }

    void Rectangle::volumeStartEvent([[maybe_unused]] const SDL_Event &e)
    {
        SDL_Event ev;
        ev.type = volumeEventData;
        ev.user.code = 0;
        ev.user.data1 = this;
        SDL_PushEvent(&ev);

        addCallback(SDL_MOUSEBUTTONUP, std::bind(Rectangle::volumeStopEvent, this, std::placeholders::_1));
        addCallback(SDL_MOUSEMOTION, std::bind(Rectangle::volumeBypass, this, std::placeholders::_1));
    }

    void Rectangle::volumeStopEvent([[maybe_unused]] const SDL_Event &e)
    {
        clearCallback(SDL_MOUSEBUTTONUP);
        clearCallback(SDL_MOUSEMOTION);
    }

} // namespace gui