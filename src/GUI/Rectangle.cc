#include "Rectangle.hh"

#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glext.h>

#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>
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
                                                                   w(std::clamp(wp, 0.0f, 1.0f)),
                                                                   h(std::clamp(hp, 0.0f, 1.0f)),
                                                                   x(std::clamp(xp, -1.0f + 1.0f * w, 1.0f - 1.0f * w)),
                                                                   y(std::clamp(yp, -1.0f + 1.0f * h, 1.0f - 1.0f * h))
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

    Rectangle::Rectangle(Rectangle &&r) : EventManager<Rectangle>(r)
    {
        volume.swap(r.volume);
        text.swap(r.text);
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

    void Rectangle::process(const SDL_Event &e)
    {
        EventManager::process(this, e);
    }

    Volume &Rectangle::allocVolume(unsigned int depth, unsigned int length, int unsigned width, const std::vector<uint8_t> &data)
    {

        if (!texture)
        {
            allocTexture(512, 512);
        }

        addCallback(SDL_MOUSEWHEEL, volumeBypass);
        addCallback(SDL_MOUSEBUTTONDOWN, volumeStartEvent);
        volume = std::make_unique<Volume>(depth, length, width, data);
        return *volume;
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
        // id = glm::rotate(id, angle);
        modelview = glm::translate(modelview, {x + offX, y + offY, 0.0});
        modelview = glm::scale(modelview, {w, h, 1.0});
        modelview *= mv;

        for (auto &r : subRectangles)
        {
            r.update(modelview);
        }
    }

    void Rectangle::render() const
    {

        glUniformMatrix4fv(modelviewUni, 1, GL_FALSE, glm::value_ptr(modelview));

        glBindTexture(GL_TEXTURE_2D, texture);

        if (volume)
        {
            glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pixelBuffer);
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, ww, hh, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, 0);
            glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
        }

        glBindVertexArray(vArray);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        glBindVertexArray(0);

        glBindTexture(GL_TEXTURE_2D, 0);

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
        addCallback(SDL_MOUSEBUTTONDOWN, dragStartEvent);
    }

    void Rectangle::setBG(SDL_Colour c)
    {
        colour = c;
    }

    Rectangle &Rectangle::addRectangle(float xx, float yy, float www, float hhh)
    {
        auto &r = subRectangles.emplace_back(xx, yy, www, hhh);
        r.update(modelview);
        r.modelviewUni = modelviewUni;
        return r;
    }

    bool Rectangle::contains([[maybe_unused]] float rx, [[maybe_unused]] float ry)
    {
        glm::mat4 inv = glm::inverse(modelview);
        glm::vec4 mvec(rx, ry, 0.0f, 1.0f);

        mvec = inv * mvec;

        if(mvec.x < -1.0f || mvec.x > 1.0f || mvec.y < -1.0f || mvec.y > 1.0f)
        {
            return false;
        }
        else
        {
            return true;
        }
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
        if (e.button.button != SDL_BUTTON_LEFT)
        {
            return;
        }

        offX = 0;
        offY = 0;

        clearCallback(SDL_MOUSEBUTTONUP);
        clearCallback(SDL_MOUSEMOTION);
        // addCallback(SDL_MOUSEBUTTONDOWN, std::bind(dragStartEvent, this, std::placeholders::_1));
    }

    void Rectangle::dragStartEvent([[maybe_unused]] const SDL_Event &e)
    {

        if (e.button.button != SDL_BUTTON_LEFT)
        {
            return;
        }

        SDL_Event ev;
        ev.type = dropEventData;
        ev.user.windowID = e.button.windowID;
        ev.user.code = 0;
        ev.user.data1 = this;
        SDL_PushEvent(&ev);

        addCallback(SDL_MOUSEBUTTONUP, Rectangle::dragStopEvent);
        addCallback(SDL_MOUSEMOTION, Rectangle::dragEvent);
        // clearCallback(SDL_MOUSEBUTTONDOWN); // Ignore Right clicks.
    }

    void Rectangle::dropEvent([[maybe_unused]] const SDL_Event &e)
    {
        
        Rectangle *rp = static_cast<Rectangle *>(e.user.data1);
        
        Rectangle &r = subRectangles.emplace_back(rp->x + rp->offX, rp->y + rp->offY, rp->w, rp->h);
        r.update(modelview);
        r.modelviewUni = modelviewUni;
        r.setBG({0xFF, 0xFF, 0xFF, 0xFF});

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
        
        if (e.button.button == SDL_BUTTON_LEFT)
        {
            ev.user.code = 0;
            volume->addCallback(SDL_MOUSEMOTION, Volume::rotateEvent);
        }
        else if (e.button.button == SDL_BUTTON_RIGHT)
        {
            ev.user.code = 1;
            volume->addCallback(SDL_MOUSEMOTION, Volume::dragEvent);
        }
        else
        {
            return;
        }
        
        ev.type = volumeEventData;
        ev.user.data1 = this;
        SDL_PushEvent(&ev);
        
        addCallback(SDL_MOUSEBUTTONUP, Rectangle::volumeStopEvent);
        addCallback(SDL_MOUSEMOTION, Rectangle::volumeBypass);
        addCallback(SDL_MOUSEBUTTONDOWN, Rectangle::doubleDown);
    }

    void Rectangle::volumeStopEvent([[maybe_unused]] const SDL_Event &e)
    {
        clearCallback(SDL_MOUSEBUTTONUP);
        clearCallback(SDL_MOUSEMOTION);
        addCallback(SDL_MOUSEBUTTONDOWN, Rectangle::volumeStartEvent);
        volume->clearCallback(SDL_MOUSEMOTION);
    }

    void Rectangle::scrollEvent(const SDL_Event &e)
    {
        std::pair<int, int> size;
        SDL_GetWindowSize(SDL_GetWindowFromID(e.wheel.windowID), &size.first, &size.second);

        int mx = 0, my = 0;
        SDL_GetMouseState(&mx, &my);

        float rx = std::lerp(x, x+w, static_cast<float>(mx)/static_cast<float>(size.first));
        float ry = std::lerp(y, y+h, 1.0f - static_cast<float>(my)/static_cast<float>(size.second));

        for (auto &r : subRectangles)
        {
            if (r.contains(rx, ry))
            {
                r.process(e);
                break;
            }
        }
    }

    void Rectangle::scaleEvent([[maybe_unused]] const SDL_Event &e)
    {
    }

    void Rectangle::doubleDown([[maybe_unused]] const SDL_Event &e)
    {
        SDL_Event ev;

        if (e.button.button == SDL_BUTTON_LEFT)
        {
            ev.user.code = 0;
            volume->addCallback(SDL_MOUSEMOTION, Volume::rotateEvent);
        }
        else if (e.button.button == SDL_BUTTON_RIGHT)
        {
            ev.user.code = 1;
            volume->addCallback(SDL_MOUSEMOTION, Volume::dragEvent);
        }
        else
        {
            return;
        }
        
        ev.type = volumeEventData;
        ev.user.data1 = this;
        
        SDL_PushEvent(&ev);

        addCallback(SDL_MOUSEBUTTONUP, Rectangle::doubleUp);
    }

    void Rectangle::doubleUp([[maybe_unused]] const SDL_Event &e)
    {
        SDL_Event ev;
        
        if (e.button.button == SDL_BUTTON_LEFT)
        {
            ev.user.code = 1;
            volume->addCallback(SDL_MOUSEMOTION, Volume::dragEvent);
        }
        else if (e.button.button == SDL_BUTTON_RIGHT)
        {
            ev.user.code = 0;
            volume->addCallback(SDL_MOUSEMOTION, Volume::rotateEvent);
        }
        else
        {
            return;
        }
        
        ev.type = volumeEventData;
        ev.user.data1 = this;
        
        SDL_PushEvent(&ev);

        addCallback(SDL_MOUSEBUTTONUP, Rectangle::volumeStopEvent);
    }

} // namespace gui