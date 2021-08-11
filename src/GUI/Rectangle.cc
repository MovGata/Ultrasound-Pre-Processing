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

#include "../Events/GUI.hh"

namespace gui
{
    // Rectangle Rectangle::arrow;
    // std::optional<glm::mat4> Rectangle::mouseArrow;
    std::once_flag Rectangle::onceFlag;

    GLuint Rectangle::vBuffer = 0;
    GLuint Rectangle::tBuffer = 0;
    GLuint Rectangle::vArray = 0;

    Rectangle::Rectangle() : Rectangle(0.0f, 0.0f, 1.0f, 1.0f) {}

    Rectangle::Rectangle(float xp, float yp, float wp, float hp, std::shared_ptr<Texture> &&t)
        : modelview(1.0f), transformations(1.0f), texture(std::forward<std::shared_ptr<Texture>>(t)),
          w(wp), h(hp), x(xp), y(yp)
    {
        std::call_once(
            onceFlag,
            [this]
            {
                glGenBuffers(1, &vBuffer);
                glBindBuffer(GL_ARRAY_BUFFER, vBuffer);
                glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);
                
                glGenBuffers(1, &tBuffer);
                glBindBuffer(GL_ARRAY_BUFFER, tBuffer);
                glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(GLfloat), texcoords.data(), GL_STATIC_DRAW);

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
            });
    }

    // std::weak_ptr<Volume> Rectangle::allocVolume(unsigned int depth, unsigned int length, int unsigned width, const std::vector<uint8_t> &data)
    // {

    //     if (!texture)
    //     {
    //         allocTexture(512, 512);
    //     }

    //     // addCallback(SDL_MOUSEWHEEL, volumeBypass);
    //     // addCallback(SDL_MOUSEBUTTONDOWN, volumeStartEvent);
    //     volume = std::make_unique<Volume>(depth, length, width, data);
    //     return volume;
    // }

    void Rectangle::update()
    {
        glm::mat4 id(1.0f);
        modelview = id;
        modelview = glm::scale(id, {w, h, 1.0}) * modelview;
        modelview = glm::rotate(id, angle, {0.0f, 0.0f, 1.0f}) * modelview;
        modelview = glm::translate(id, {x, y, 0.0}) * modelview;
    }

    void Rectangle::update(float xx, float yy)
    {
        x = xx;
        y = yy;
        update();
    }

    void Rectangle::update(float xx, float yy, float ww, float hh)
    {
        w = ww;
        h = hh;
        update(xx, yy);
    }

    void Rectangle::drawChildren() const
    {
        if (!hidden)
        {
            return;
        }

        // for (const auto &r : subRectangles)
        // {
        //     events::visible(events::draw<Rectangle>, *r);
        // }

        // for (const auto &r : subRectangles)
        // {
        //     r->drawChildren();
        // }
    }

    // void Rectangle::drawLinks()
    // {
    //     if (!hidden)
    //     {
    //         return;
    //     }

    //     std::erase_if(outlinks, [](const std::pair<std::weak_ptr<Rectangle>, std::shared_ptr<glm::mat4>> &p)
    //                   { return p.first.expired(); });

    //     std::erase_if(inlinks, [](const std::pair<std::weak_ptr<Rectangle>, std::shared_ptr<glm::mat4>> &p)
    //                   { return p.first.expired(); });

    //     for (const auto &r : outlinks)
    //     {
    //         arrow.transformations = *r.second;
    //         arrow.draw();
    //     }

    //     for (const auto &r : subRectangles)
    //     {
    //         r->drawLinks();
    //     }
    // }

    // void Rectangle::addText(TTF_Font *f, const std::string &str)
    // {
    //     font = f;
    //     SDL_Surface *s = TTF_RenderText_Blended(font, str.c_str(), SDL_Colour{0xFF, 0xFF, 0xFF, 0xFF});
    //     if (!s)
    //     {
    //         std::cerr << "Failed to create text: " << TTF_GetError() << '\n';
    //     }
    //     SDL_Surface *textT = SDL_ConvertSurfaceFormat(s, SDL_PixelFormatEnum::SDL_PIXELFORMAT_RGBA8888, 0);
    //     SDL_FreeSurface(s);
    //     if (texture == 0)
    //     {
    //         allocTexture(textT->w, textT->h);
    //     }

    //     glBindTexture(GL_TEXTURE_2D, *texture);
    //     if (textT->w * textT->h < textureW * textureH)
    //     {

    //         std::vector<uint32_t> clr;
    //         clr.reserve(textureW * textureH);
    //         std::fill_n(std::back_inserter(clr), textureW * textureH, static_cast<uint32_t>(colour.r) << 24 | static_cast<uint32_t>(colour.g) << 16 | static_cast<uint32_t>(colour.b) << 8 | static_cast<uint32_t>(colour.a));
    //         glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, textureW, textureH, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, clr.data());
    //         glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, textT->w, textT->h, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, textT->pixels);
    //     }
    //     else
    //     {
    //         glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, textureW, textureH, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8, textT->pixels);
    //     }

    //     glBindTexture(GL_TEXTURE_2D, 0);
    //     text = str;

    //     SDL_FreeSurface(textT);
    // }

    // void Rectangle::setBG(SDL_Colour c)
    // {
    //     colour = c;
    // }

    // std::weak_ptr<Rectangle> Rectangle::addRectangle(float xPos, float yPos, float width, float height)
    // {
    //     // auto &r = subRectangles.emplace_back(std::make_shared<Rectangle>(xPos, yPos, width, height));
    //     r->update(modelview);
    //     // r->modelviewUni = modelviewUni;
    //     return r;
    // }

    // bool Rectangle::contains(float rx, float ry) const
    // {
    //     glm::mat4 inv = glm::inverse(modelview);
    //     glm::vec4 mvec(rx, ry, 0.0f, 1.0f);

    //     mvec = inv * mvec;

    //     if (mvec.x < -1.0f || mvec.x > 1.0f || mvec.y < -1.0f || mvec.y > 1.0f)
    //     {
    //         return false;
    //     }
    //     else
    //     {
    //         return true;
    //     }
    // }

    // bool Rectangle::containsTF(float rx, float ry) const
    // {
    //     glm::mat4 inv = glm::inverse(transformations * modelview);
    //     glm::vec4 mvec(rx, ry, 0.0f, 1.0f);

    //     mvec = inv * mvec;

    //     if (mvec.x < -1.0f || mvec.x > 1.0f || mvec.y < -1.0f || mvec.y > 1.0f)
    //     {
    //         return false;
    //     }
    //     else
    //     {
    //         return true;
    //     }
    // }

    // bool Rectangle::contains(const Rectangle &rec) const
    // {
    //     if (&rec == this)
    //     {
    //         return false;
    //     }

    //     glm::vec4 bl(-1.0f, -1.0f, 0.0f, 1.0f);
    //     bl = rec.modelview * bl;

    //     glm::vec4 tr(1.0f, 1.0f, 0.0f, 1.0f);
    //     tr = rec.modelview * tr;

    //     return contains(bl.x, bl.y) && contains(tr.x, tr.y);
    // }

    // bool Rectangle::containsTF(const Rectangle &rec) const
    // {
    //     if (&rec == this)
    //     {
    //         return false;
    //     }

    //     glm::vec4 bl(-1.0f, -1.0f, 0.0f, 1.0f);
    //     bl = rec.transformations * rec.modelview * bl;

    //     glm::vec4 tr(1.0f, 1.0f, 0.0f, 1.0f);
    //     tr = rec.transformations * rec.modelview * tr;

    //     return containsTF(bl.x, bl.y) && containsTF(tr.x, tr.y);
    // }

    // glm::vec4 Rectangle::snapToEdge(const glm::vec4 &v) const
    // {
    //     glm::vec4 base = glm::inverse(modelview) * v;
    //     base.x = (base.x > 1.0f || base.x < -1.0f) ? std::clamp(base.x, -1.0f, 1.0f) : base.x;
    //     base.y = (base.y > 1.0f || base.y < -1.0f) ? std::clamp(base.y, -1.0f, 1.0f) : base.y;
    //     return modelview * base; // Back to global
    // }

    // DRAG AND DROP EVENTS

    // void Rectangle::dragEvent(const SDL_Event &e)
    // {
    //     int width, height;
    //     SDL_GetWindowSize(SDL_GetWindowFromID(e.motion.windowID), &width, &height);
    //     float X = static_cast<float>(2 * e.motion.xrel) / static_cast<float>(width);
    //     float Y = static_cast<float>(2 * e.motion.yrel) / static_cast<float>(-height);
    //     transformations = glm::translate(glm::mat4(1.0f), {X, Y, 0.0f}) * transformations;
    // }

    // void Rectangle::dragStopEvent([[maybe_unused]] const SDL_Event &e)
    // {
    //     if (e.button.button != SDL_BUTTON_LEFT)
    //     {
    //         return;
    //     }

    //     // clearCallback(SDL_MOUSEBUTTONUP);
    //     // clearCallback(SDL_MOUSEMOTION);
    //     // addCallback(SDL_MOUSEBUTTONDOWN, std::bind(dragStartEvent, this, std::placeholders::_1));
    //     transformations = glm::mat4(1.0f);
    // }

    // void Rectangle::dragStartEvent([[maybe_unused]] const SDL_Event &e)
    // {
    //     if (e.button.button != SDL_BUTTON_LEFT)
    //     {
    //         return;
    //     }

    // SDL_Event ev;
    // ev.type = dropEventData;
    // ev.user.windowID = e.button.windowID;
    // ev.user.code = DRAG_FILTER;
    // ev.user.data1 = this;
    // SDL_PushEvent(&ev);

    // addCallback(SDL_MOUSEBUTTONUP, Rectangle::dragStopEvent);
    // addCallback(SDL_MOUSEMOTION, Rectangle::dragEvent);
    // clearCallback(SDL_MOUSEBUTTONDOWN); // Ignore Right clicks.
    // }

    // void Rectangle::dropEvent([[maybe_unused]] const SDL_Event &e)
    // {
    //     if (e.user.code == DRAG_FILTER)
    //     {
    //         Rectangle *rp = static_cast<Rectangle *>(e.user.data1);
    //         std::shared_ptr<Rectangle> r = subRectangles.emplace_back(std::make_shared<Rectangle>(*rp));

    //         glm::mat4 loc = glm::inverse(modelview) * r->transformations * r->modelview;

    //         r->x = loc[3][0];
    //         r->y = loc[3][1];

    //         r->w = loc[0][0];
    //         r->h = loc[1][1];

    //         r->update(modelview);
    //         r->transformations = glm::mat4(1.0f);

    //         r->clearCallbacks();
    //         // r->addCallback(SDL_MOUSEBUTTONDOWN, moveStartEvent);
    //     }
    //     else if (e.user.code == DRAG_LINK)
    //     {
    //         glm::vec4 mouse = globalMouse(e.user.windowID);

    //         Rectangle *rp = static_cast<Rectangle *>(e.user.data1);
    //         for (auto &r : subRectangles)
    //         {
    //             if (r.get() != rp && r->contains(mouse.x, mouse.y) && !r->process(e))
    //             {
    //                 glm::vec4 origin = {0.0f, 0.0f, 0.0f, 1.0f};
    //                 glm::vec4 base = rp->snapToEdge(r->modelview * origin);
    //                 glm::vec4 tip = r->snapToEdge(rp->modelview * origin);

    //                 float W = (tip.x - base.x);
    //                 float Y = (tip.y - base.y);

    //                 glm::mat4 id(1.0f);
    //                 glm::mat4 linkArrow(1.0f);
    //                 linkArrow = glm::scale(id, {glm::distance(tip, base), 1.0f, 1.0f});
    //                 linkArrow = glm::rotate(id, std::atan2(Y, W), {0.0f, 0.0f, 1.0f}) * linkArrow;
    //                 linkArrow = glm::translate(id, {base.x, base.y, 0.0f}) * linkArrow;

    //                 // rp->outlinks.emplace_back(r->weak_from_this(), std::make_shared<glm::mat4>(linkArrow));
    //                 // r->inlinks.emplace_back(rp->weak_from_this(), rp->outlinks.back().second);
    //                 break;
    //             }
    //         }
    //     }
    // }

    // void Rectangle::moveEvent(const SDL_Event &e)
    // {
    //     dragEvent(e);
    // for (const auto &link : inlinks)
    // {
    //     std::shared_ptr in = link.first.lock();
    //     glm::vec4 origin = {0.0f, 0.0f, 0.0f, 1.0f};
    //     glm::vec4 base = in->snapToEdge(modelview * origin);
    //     glm::vec4 tip = snapToEdge(in->modelview * origin);

    //     float W = (tip.x - base.x);
    //     float Y = (tip.y - base.y);

    //     glm::mat4 id(1.0f);
    //     *link.second = glm::scale(id, {glm::distance(tip, base), 1.0f, 1.0f});
    //     *link.second = glm::rotate(id, std::atan2(Y, W), {0.0f, 0.0f, 1.0f}) * *link.second;
    //     *link.second = glm::translate(id, {base.x, base.y, 0.0f}) * *link.second;
    // }

    // for (const auto &link : outlinks)
    // {
    //     std::shared_ptr out = link.first.lock();
    //     glm::vec4 origin = {0.0f, 0.0f, 0.0f, 1.0f};
    //     glm::vec4 base = snapToEdge(out->modelview * origin);
    //     glm::vec4 tip = out->snapToEdge(modelview * origin);

    //     float W = (tip.x - base.x);
    //     float Y = (tip.y - base.y);

    //     glm::mat4 id(1.0f);
    //     *link.second = glm::scale(id, {glm::distance(tip, base), 1.0f, 1.0f});
    //     *link.second = glm::rotate(id, std::atan2(Y, W), {0.0f, 0.0f, 1.0f}) * *link.second;
    //     *link.second = glm::translate(id, {base.x, base.y, 0.0f}) * *link.second;
    // }
    // }

    // void Rectangle::moveStopEvent(const SDL_Event &e)
    // {
    //     if (e.button.button == SDL_BUTTON_LEFT)
    //     {
    //         modelview = transformations * modelview;

    //         w = modelview[0][0];
    //         h = modelview[1][1];
    //         x = modelview[3][0];
    //         y = modelview[3][1];

    //         transformations = glm::mat4(1.0f);
    //     }
    //     else if (e.button.button != SDL_BUTTON_RIGHT)
    //     {
    //         return;
    //     }

    //     mouseArrow.reset();
    //     clearCallback(SDL_MOUSEBUTTONUP);
    //     clearCallback(SDL_MOUSEMOTION);
    // }

    // void Rectangle::moveStartEvent(const SDL_Event &e)
    // {
    //     SDL_Event ev;
    //     ev.user.windowID = e.button.windowID;
    //     ev.user.data1 = this;
    //     if (e.button.button == SDL_BUTTON_LEFT)
    //     {
    //         // ev.type = moveEventData;
    //         ev.user.code = MOVE_FILTER;
    //         // addCallback(SDL_MOUSEMOTION, Rectangle::moveEvent);
    //     }
    //     else if (e.button.button == SDL_BUTTON_RIGHT)
    //     {
    //         // ev.type = dropEventData;
    //         ev.user.code = DRAG_LINK;
    //         // addCallback(SDL_MOUSEMOTION, Rectangle::updateArrow);
    //     }
    //     // addCallback(SDL_MOUSEBUTTONUP, Rectangle::moveStopEvent);
    //     SDL_PushEvent(&ev);
    // }

    // void Rectangle::stopEvent(const SDL_Event &e)
    // {
    //     Rectangle *rec = static_cast<Rectangle *>(e.user.data1);
    //     bool child = false;
    //     for (auto &r : subRectangles)
    //     {
    //         if (r.get() == rec)
    //         {
    //             child = true;
    //             break;
    //         }

    //         if (r->contains(*rec))
    //         {
    //             r->process(e);
    //             break;
    //         }
    //     }

    //     // Optimise via weak pointers.
    //     if (child && !containsTF(*rec))
    //     {
    //         std::erase_if(subRectangles, [rec](const std::shared_ptr<Rectangle> &r)
    //                       { return r.get() == rec; });
    //     }
    // }

    // void Rectangle::updateArrow(const SDL_Event &e)
    // {
    //     glm::vec4 mouse = globalMouse(e.motion.windowID);
    //     glm::vec4 base = snapToEdge(mouse);

    //     float W = (mouse.x - base.x);
    //     float Y = (mouse.y - base.y);

    //     glm::mat4 id(1.0f);
    //     mouseArrow = glm::scale(id, {glm::distance(mouse, base), 1.0f, 1.0f});
    //     mouseArrow = glm::rotate(id, std::atan2(Y, W), {0.0f, 0.0f, 1.0f}) * *mouseArrow;
    //     mouseArrow = glm::translate(id, {base.x, base.y, 0.0f}) * *mouseArrow;
    // }

    // void Rectangle::hideEvent(const SDL_Event &e)
    // {
    //     SDL_Event ev;
    //     // ev.user.type = showEventData;
    //     ev.user.windowID = e.button.windowID;
    //     ev.user.code = 0;
    //     ev.user.data1 = &text;
    //     ev.user.data2 = nullptr;

    //     SDL_PushEvent(&ev);
    // }

    // void Rectangle::showEvent(const SDL_Event &e)
    // {
    //     std::string str = *static_cast<std::string *>(e.user.data1);
    //     if (str == text)
    //     {
    //         hidden = true;
    //     }
    //     else
    //     {
    //         hidden = false;
    //     }
    // }

    // VOLUME EVENTS

    // void Rectangle::volumeBypass(const SDL_Event &e)
    // {
    //     volume->process(e);
    // }

    // void Rectangle::volumeStartEvent([[maybe_unused]] const SDL_Event &e)
    // {

    // SDL_Event ev;

    // if (e.button.button == SDL_BUTTON_LEFT)
    // {
    //     ev.user.code = VOLUME_ROTATE;
    //     volume->addCallback(SDL_MOUSEMOTION, Volume::rotateEvent);
    // }
    // else if (e.button.button == SDL_BUTTON_RIGHT)
    // {
    //     ev.user.code = VOLUME_DRAG;
    //     volume->addCallback(SDL_MOUSEMOTION, Volume::dragEvent);
    // }
    // else
    // {
    //     return;
    // }

    // ev.user.type = volumeEventData;
    // ev.user.windowID = e.button.windowID;
    // ev.user.data1 = this;
    // SDL_PushEvent(&ev);

    // addCallback(SDL_MOUSEBUTTONUP, Rectangle::volumeStopEvent);
    // addCallback(SDL_MOUSEMOTION, Rectangle::volumeBypass);
    // addCallback(SDL_MOUSEBUTTONDOWN, Rectangle::doubleDown);
    // }

    // void Rectangle::volumeStopEvent([[maybe_unused]] const SDL_Event &e)
    // {
    //     clearCallback(SDL_MOUSEBUTTONUP);
    //     clearCallback(SDL_MOUSEMOTION);
    //     // addCallback(SDL_MOUSEBUTTONDOWN, Rectangle::volumeStartEvent);
    //     volume->clearCallback(SDL_MOUSEMOTION);
    // }

    // void Rectangle::scrollEvent(const SDL_Event &e)
    // {
    //     std::pair<int, int> size;
    //     SDL_GetWindowSize(SDL_GetWindowFromID(e.wheel.windowID), &size.first, &size.second);

    //     int mx = 0, my = 0;
    //     SDL_GetMouseState(&mx, &my);

    //     float rx = std::lerp(x, x + w, static_cast<float>(mx) / static_cast<float>(size.first));
    //     float ry = std::lerp(y, y + h, 1.0f - static_cast<float>(my) / static_cast<float>(size.second));

    //     for (auto &r : subRectangles)
    //     {
    //         if (r->contains(rx, ry))
    //         {
    //             r->process(e);
    //             break;
    //         }
    //     }
    // }

    // void Rectangle::scaleEvent([[maybe_unused]] const SDL_Event &e)
    // {
    // }

    // void Rectangle::doubleDown([[maybe_unused]] const SDL_Event &e)
    // {
    //     SDL_Event ev;

    //     // if (e.user.code == VOLUME_DRAG)
    //     // {
    //     //     ev.user.code = VOLUME_ROTATE;
    //     //     volume->addCallback(SDL_MOUSEMOTION, Volume::rotateEvent);
    //     // }
    //     // else if (e.user.code == VOLUME_ROTATE)
    //     // {
    //     //     ev.user.code = VOLUME_DRAG;
    //     //     volume->addCallback(SDL_MOUSEMOTION, Volume::dragEvent);
    //     // }
    //     // else
    //     // {
    //     //     return;
    //     // }

    //     // ev.type = volumeEventData;
    //     ev.user.data1 = this;

    //     SDL_PushEvent(&ev);

    //     // addCallback(SDL_MOUSEBUTTONUP, Rectangle::doubleUp);
    // }

    // void Rectangle::doubleUp([[maybe_unused]] const SDL_Event &e)
    // {
    //     SDL_Event ev = e;

    //     // if (e.user.code == VOLUME_ROTATE)
    //     // {
    //     //     ev.user.code = VOLUME_DRAG;
    //     //     volume->addCallback(SDL_MOUSEMOTION, Volume::dragEvent);
    //     // }
    //     // else if (e.user.code == VOLUME_DRAG)
    //     // {
    //     //     ev.user.code = VOLUME_ROTATE;
    //     //     volume->addCallback(SDL_MOUSEMOTION, Volume::rotateEvent);
    //     // }
    //     // else
    //     // {
    //     //     return;
    //     // }

    //     SDL_PushEvent(&ev);

    //     // addCallback(SDL_MOUSEBUTTONUP, Rectangle::volumeStopEvent);
    // }

} // namespace gui