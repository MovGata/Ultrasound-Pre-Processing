#ifndef EVENTS_GUI_HH
#define EVENTS_GUI_HH

#include <concepts>
#include <cmath>
#include <iostream>

#include <glm/ext.hpp>

#include "Concepts.hh"
#include "../GUI/Instance.hh"

namespace events
{
    const Uint32 GUI_SHOW = SDL_RegisterEvents(7);
    const Uint32 GUI_DROP = GUI_SHOW + 1;
    const Uint32 GUI_MOVE = GUI_SHOW + 2;
    const Uint32 GUI_VOLUME = GUI_SHOW + 3;
    const Uint32 GUI_DRAW = GUI_SHOW + 4;
    const Uint32 GUI_TOGGLE = GUI_SHOW + 5;
    const Uint32 GUI_REDRAW = GUI_SHOW + 6;

    constexpr Uint32 SHOWEVENT_HIDE = 0;
    constexpr Uint32 SHOWEVENT_SHOW = 1;

    constexpr std::size_t DRAG_FILTER = 0;
    constexpr std::size_t DRAG_LINK = 1;

    constexpr std::size_t MOVE_FILTER = 0;

    constexpr std::size_t VOLUME_ROTATE = 0;
    constexpr std::size_t VOLUME_DRAG = 1;

    constexpr Uint32 TOGGLEEVENT_OFF = 0;
    constexpr Uint32 TOGGLEEVENT_ON = 1;

    constexpr Uint32 DROPEVENT_KERNEL = 0;
    constexpr Uint32 DROPEVENT_LINK = 1;

    template <concepts::DrawableType Drawable>
    void draw(Drawable &d)
    {
        if constexpr (concepts::TransformableType<Drawable>)
        {
            glUniformMatrix4fv(gui::Instance::modelviewUni, 1, GL_FALSE, glm::value_ptr(d.transformations * d.modelview));
        }
        else
        {
            glUniformMatrix4fv(gui::Instance::modelviewUni, 1, GL_FALSE, glm::value_ptr(d.modelview));
        }

        glBindTexture(GL_TEXTURE_2D, *d.texture);

        glBindVertexArray(d.vArray);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        glBindVertexArray(0);

        glBindTexture(GL_TEXTURE_2D, 0);

        GLint err = 0;
        if ((err = glGetError()))
        {
            std::cerr << "Draw error: " << err << '\n';
        }
    }

    template <concepts::DrawableType Drawable>
    bool containsMouse(const Drawable &d, const SDL_Event &e)
    {
        // Uint32 ID = 0;
        // switch (e.type)
        // {
        // case SDL_WINDOWEVENT:
        //     ID = e.window.windowID;
        //     break;
        // case SDL_KEYDOWN:
        // case SDL_KEYUP:
        //     ID = e.key.windowID;
        //     break;
        // case SDL_TEXTEDITING:
        //     ID = e.edit.windowID;
        //     break;
        // case SDL_TEXTINPUT:
        //     ID = e.text.windowID;
        //     break;
        // case SDL_MOUSEMOTION:
        //     ID = e.motion.windowID;
        //     break;
        // case SDL_MOUSEBUTTONDOWN:
        // case SDL_MOUSEBUTTONUP:
        //     ID = e.button.windowID;
        //     break;
        // case SDL_MOUSEWHEEL:
        //     ID = e.wheel.windowID;
        //     break;
        // case SDL_USEREVENT:
        //     ID = e.user.windowID;
        //     break;
        
        // default:
        //     return false;
        // }

        // int width, height;
        // SDL_GetWindowSize(SDL_GetWindowFromID(ID), &width, &height);

        int mx, my;
        if (e.type == SDL_MOUSEBUTTONDOWN || e.type == SDL_MOUSEBUTTONUP)
        {
            mx = e.button.x;
            my = e.button.y;
        }
        else if(e.type == SDL_MOUSEMOTION)
        {
            mx = e.motion.x;
            my = e.motion.y;
        }
        else
        {
            SDL_GetMouseState(&mx, &my);
        }

        glm::mat4 inv = glm::inverse(d.modelview);
        glm::vec4 mvec(static_cast<float>(mx), static_cast<float>(my), 0.0f, 1.0f);

        mvec = inv * mvec;

        if (mvec.x < 0.0f || mvec.x > 1.0f || mvec.y < 0.0f || mvec.y > 1.0f)
        {
            return false;
        }
        else
        {
            return true;
        }
    }

    template<concepts::DrawableType A, concepts::DrawableType B>
    bool contains(A a, B b)
    {
        return true;
    }

    template<concepts::DrawableType Drawable>
    void scaleTexture(Drawable d)
    {
        auto c = d.vertices;

    }

    template <typename F, concepts::HidableType Hidable, typename... Args>
    requires std::invocable<F, Hidable, SDL_Event, Args...>
    auto visible(F f, const SDL_Event e, const Hidable &h, Args... args) -> decltype(f(h, e, args...))
    {
        if (!h.hidden)
        {
            return f(h, e, args...);
        }
        return decltype(f(h, e, args...))();
    }

    // template <concepts::HidableType Hidable>
    // void draw(const Hidable &h, [[maybe_unsed]] const SDL_Event &e)
    // {
    //     visible(draw<concepts::DrawableType<Hidable>>, h, e);
    // }

    template <concepts::ExternalType External>
    void transfer(const External &ext, const SDL_Event &e)
    {
        glBindTexture(GL_TEXTURE_2D, *ext.texture);

        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, *ext.pixelBuffer);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, ext.textureW, ext.textureH, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, 0);
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

        glBindTexture(GL_TEXTURE_2D, 0);
    }

    template <concepts::LazyType Lazy, typename F, typename... Args>
    requires std::invocable<F, Lazy, SDL_Event, Args...>
    auto defer(F &f, const SDL_Event &e, Lazy &l, Args... args) -> decltype(f(l, e, args...))
    {
        if (l.modified)
        {
            return f(l, e, args...);
        }
        return decltype(f(l, e, args...))();
    }

    template <concepts::HidableType Hidable>
    void hide(Hidable &h, const SDL_Event &e)
    {
        h.hidden = e.user.code == SHOWEVENT_HIDE ? true : false;
    }

    template <concepts::TogglableType Toggle>
    void toggle(const SDL_Event &e, Toggle &t)
    {
        t.toggle = e.user.code == TOGGLEEVENT_ON ? true : false;
    }

    template <concepts::TogglableType Toggle, typename F, typename... Args>
    requires std::invocable<F, Toggle, Args...>
    auto toggled(F &f, const SDL_Event &e, Toggle &t, Args... args) -> decltype(f(t, e, args...))
    {
        if (t.toggle)
        {
            return f(t, e, args...);
        }
        return decltype(f(t, e, args...))();
    }

#pragma region Transformations

    template <concepts::ScalableType S>
    void scale(S &s, const glm::vec3 &v)
    {
        s.scale.x += static_cast<float>(v.x);
        s.scale.y += static_cast<float>(v.y);
        s.scale.z += static_cast<float>(v.z);

        if constexpr (concepts::LazyType<S>)
        {
            s.modified = true;
        }
    }

    template <concepts::ScalableType S>
    void scaleEvent(const SDL_Event &e, S &s)
    {
        scale(s, {e.wheel.y, e.wheel.y, e.wheel.y});
    }

    template <concepts::RotatableType R>
    void rotate(R &r, const glm::vec3 &v)
    {
        r.rotation.x = std::fmod(r.rotation.x + static_cast<float>(v.x), 360.0f);
        r.rotation.y = std::fmod(r.rotation.y + static_cast<float>(v.y), 360.0f);
        r.rotation.z = std::fmod(r.rotation.z + static_cast<float>(v.z), 360.0f);

        if constexpr (concepts::LazyType<R>)
        {
            r.modified = true;
        }
    }

    template <concepts::RotatableType R>
    void rotateEvent(const SDL_Event &e, R &r)
    {
        rotate(r, {-e.motion.yrel, e.motion.xrel, 0.0f});
    }

    template <concepts::TranslatableType T>
    void translate(T &t, const glm::vec3 &v)
    {
        t.translation.x += v.x;
        t.translation.y += v.y;
        t.translation.z += v.z;

        if constexpr (concepts::LazyType<T>)
        {
            t.modified = true;
        }
    }

    template <concepts::TranslatableType T>
    void translateEvent(const SDL_Event &e, T &t)
    {
        translate(t, {e.motion.xrel, -e.motion.yrel, 0.0f});
    }

#pragma endregion Transformations

} // namespace events

#endif