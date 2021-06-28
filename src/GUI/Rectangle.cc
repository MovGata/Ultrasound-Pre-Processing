#include "Rectangle.hh"

#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glext.h>
#include <glm/gtx/matrix_transform_2d.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

namespace gui
{
    const Uint32 Rectangle::dropEventData = SDL_RegisterEvents(2);
    const Uint32 Rectangle::volumeEventData = Rectangle::dropEventData + 1;

    Rectangle::Rectangle(float xp, float yp, float wp, float hp) : //text(nullptr, SDL_FreeSurface),
                                                                   x(std::clamp(xp, -1.0f + 1.0f * w, 1.0f - 1.0f * w)),
                                                                   y(std::clamp(yp, -1.0f + 1.0f * h, 1.0f - 1.0f * h)),
                                                                   w(std::clamp(wp, 0.0f, 1.0f)),
                                                                   h(std::clamp(hp, 0.0f, 1.0f))
    {
        std::array<GLfloat, 8> vb = {-1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f};
        glGenBuffers(1, &vBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, vBuffer);
        glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(GLfloat), vb.data(), GL_STATIC_DRAW);

        std::array<GLfloat, 8> tb = {0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f};
        glGenBuffers(1, &tBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, tBuffer);
        glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(GLfloat), tb.data(), GL_STATIC_DRAW);

        glGenVertexArrays(1, &vArray);
        glBindVertexArray(vArray);

        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, vBuffer);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, tBuffer);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        modelview = glm::mat4(1.0f);
    }

    Rectangle::Rectangle(Rectangle &&r)
    {
        volume.swap(r.volume);
        text.swap(r.text);
        events.swap(r.events);
        colour = r.colour;
        font = r.font;
        subRectangles.swap(r.subRectangles);
        modelview = std::move(r.modelview);
        angle = r.angle;
        modelviewUni = r.modelviewUni;
        vBuffer = r.vBuffer;
        tBuffer = r.tBuffer;
        vArray = r.vArray;
        texture = r.texture;
        pixelBuffer = r.pixelBuffer;
        ww = r.ww;
        hh = r.hh;
        x = r.x;
        y = r.y;
        w = r.w;
        h = r.h;
        offX = r.offX;
        offY = r.offY;

        r.vArray = 0;
        r.vBuffer = 0;
        r.tBuffer = 0;
        r.texture = 0;
        r.pixelBuffer = 0;
    }

    Rectangle::~Rectangle()
    {
        if (vArray)
        {
            glBindVertexArray(vArray);
            glDisableVertexAttribArray(0);
            glDisableVertexAttribArray(1);
            glBindVertexArray(0);
            glDeleteVertexArrays(1, &vArray);
        }

        glDeleteBuffers(1, &vBuffer);
        glDeleteBuffers(1, &tBuffer);

        glDeleteTextures(1, &texture);
        glDeleteBuffers(1, &pixelBuffer);
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

        std::vector<uint32_t> clr;
        clr.reserve(wp * hp);
        std::fill_n(std::back_inserter(clr), wp * hp, static_cast<uint32_t>(colour.r) << 24 | static_cast<uint32_t>(colour.g) << 16 | static_cast<uint32_t>(colour.b) << 8 | static_cast<uint32_t>(colour.a));
        // Texture to render PBO into a quad
        

        glGenTextures(1, &texture);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, wp, hp, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, clr.data());
        glBindTexture(GL_TEXTURE_2D, 0);

        GLenum err;
        if ((err = glGetError()))
            std::cout << "Texture err: " << err << std::endl;


        // PBO for OpenCL to render into
        glGenBuffers(1, &pixelBuffer);
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pixelBuffer);
        glBufferData(GL_PIXEL_UNPACK_BUFFER, wp * hp * sizeof(GLubyte) * 4, clr.data(), GL_STREAM_DRAW);
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

        if ((err = glGetError()))
            std::cout << "Pixel err: " << err << std::endl;
    }

    void Rectangle::update(const glm::mat4 &mv)
    {
        glm::mat4 id(1.0f);
        modelview = id;
        // modelview *= ;
        // id = glm::rotate(id, angle);
        modelview = glm::translate(modelview, {x + offX, y + offY, 0.0});
        
        auto ptr = glm::value_ptr(modelview);
        for (int i = 0; i < 16; ++i )
        {
            std::cout << ptr[i] << ' ';
        }
        std::cout << std::endl;
        
        modelview = glm::scale(modelview, {w, h, 1.0});
        
        ptr = glm::value_ptr(modelview);
        for (int i = 0; i < 16; ++i )
        {
            std::cout << ptr[i] << ' ';
        }
        std::cout << std::endl;
        modelview *= mv;


        for (auto &r : subRectangles)
        {
            r.update(modelview);
        }
    }

    void Rectangle::render() const
    {
        // float xlerp = std::lerp(xp, xp + 2.0f * wp, (x + 1.0f) / 2.0f);
        // float ylerp = std::lerp(yp, yp + 2.0f * hp, (y + 1.0f) / 2.0f);
        // float wlerp = xlerp + std::lerp(xp, xp + 2.0f * wp, w / 2.0f) + 1.0f;
        // float hlerp = ylerp + std::lerp(yp, yp + 2.0f * hp, h / 2.0f) + 1.0f;

        // glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pixelBuffer);
        glUniformMatrix4fv(modelviewUni, 1, GL_FALSE, glm::value_ptr(modelview));
        glBindTexture(GL_TEXTURE_2D, texture);
        // glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, ww, hh, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, 0);


        glBindVertexArray(vArray);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        glBindVertexArray(0);

        glBindTexture(GL_TEXTURE_2D, 0);
        // glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

        GLint err = 0;
        if ((err = glGetError()))
        {
            std::cerr << "Rectangle render error: " << err << '\n';
        }


        for (const auto &r : subRectangles)
        {
            r.render();
        }
    }

    void Rectangle::addText(TTF_Font *f, const std::string &str)
    {
        font = f;
        SDL_Surface *s = TTF_RenderText_Blended(font, str.c_str(), SDL_Colour{0xFF, 0xFF, 0xFF, 0xFF});
        if (!s)
        {
            std::cerr << "Failed to create text: " << TTF_GetError() << '\n';
        }
        SDL_Surface *textT = SDL_ConvertSurfaceFormat(s, SDL_PixelFormatEnum::SDL_PIXELFORMAT_RGBA8888, 0);
        SDL_FreeSurface(s);
        if (texture == 0)
        {
            allocTexture(textT->w, textT->h);
        }

        // glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pixelBuffer);
        glBindTexture(GL_TEXTURE_2D, texture);
        if (textT->w * textT->h < ww * hh)
        {

            std::vector<uint32_t> clr;
            clr.reserve(ww * hh);
            std::fill_n(std::back_inserter(clr), ww * hh, static_cast<uint32_t>(colour.r) << 24 | static_cast<uint32_t>(colour.g) << 16 | static_cast<uint32_t>(colour.b) << 8 | static_cast<uint32_t>(colour.a));
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, ww, hh, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, clr.data());
            // for (auto i = 0; i < textT->h; ++i)
            // {
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, textT->w, textT->h, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, textT->pixels);
                // glBufferSubData(GL_PIXEL_UNPACK_BUFFER, i * ww * 4, textT->pitch, static_cast<uint32_t *>(textT->pixels) + i * textT->w);
            // }
        }
        else
        {
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, ww, hh, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8, textT->pixels);
            // glBufferSubData(GL_PIXEL_UNPACK_BUFFER, 0, (ww * hh) * sizeof(GLubyte) * 4, textT->pixels);
        }
        // glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);


        // glBindTexture(GL_TEXTURE_2D, texture);
        glBindTexture(GL_TEXTURE_2D, 0);
        text = str;

        SDL_FreeSurface(textT);
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
        r.update(modelview);
        r.modelviewUni = modelviewUni;
        r.setBG({0xFF, 0xFF, 0xFF, 0xFF});
        // r.allocTexture(rp->ww, rp->hh);
        // r.text = rp->text;
        // r.addText(rp->font, r.text);
        r.texture = rp->texture;
        r.pixelBuffer = rp->pixelBuffer;
        r.ww = rp->ww;
        r.hh = rp->hh;
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

    void Rectangle::scrollEvent([[maybe_unused]] const SDL_Event &e)
    {
    }

    void Rectangle::scaleEvent([[maybe_unused]] const SDL_Event &e)
    {
    }

} // namespace gui