#include <cmath>
#include <iostream>
#include <numbers>

#include <gl/glew.h>
#include <gl/gl.h>
#include <gl/glext.h>

#include "Window.hh"

namespace gui
{

    Window::Window(unsigned int w, unsigned int h, Uint32 flags) : Window(SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, w, h, flags) {}
    Window::Window(unsigned int x, unsigned int y, unsigned int w, unsigned int h, Uint32 flags) : window(nullptr, SDL_DestroyWindow)
    {

        window.reset(SDL_CreateWindow("Ultrasound Pre-Processor", x, y, w, h, flags));
        if (window == nullptr)
        {
            std::cout << "SDL2 window initialisation failed, error: " << SDL_GetError() << std::endl;
            return;
        }

        addCallback(SDL_WINDOWEVENT, std::bind(Window::processWindowEvent, this, std::placeholders::_1));
        addCallback(SDL_MOUSEWHEEL, std::bind(Window::mouseScroll, this, std::placeholders::_1));
        addCallback(SDL_MOUSEBUTTONUP, std::bind(Window::mouseClick, this, std::placeholders::_1));
        addCallback(SDL_MOUSEBUTTONDOWN, std::bind(Window::mouseClick, this, std::placeholders::_1));
        addCallback(SDL_MOUSEMOTION, std::bind(Window::mouseMotion, this, std::placeholders::_1));
        addCallback(Rectangle::dropEvent, std::bind(Window::userDrop, this, std::placeholders::_1));

        // surface.reset(SDL_GetWindowSurface(window.get()));

        glContext = SDL_GL_CreateContext(window.get());
        if (glContext == nullptr)
        {
            std::cout << "OpenGL context initialisation failed, error: " << SDL_GetError() << std::endl;
            window.release();
            return;
        }

        GLenum gError = glewInit();
        if (gError != GLEW_OK)
        {
            std::cout << "Glew init error: " << glewGetErrorString(gError) << std::endl;
            window.release();
            return;
        }

        if (!GLEW_VERSION_2_1)
        {
            std::cout << "OpenGL 2.1 minimum" << std::endl;
            window.release();
            return;
        }

        // Vsync
        if (SDL_GL_SetSwapInterval(1) < 0)
        {
            std::cout << "VSync unavailable: " << SDL_GetError() << std::endl;
        }

        glDisable(GL_DEPTH_TEST);
        clean();
        redraw();
    }

    Window::~Window()
    {
    }

    auto Window::getPosition() -> std::pair<int, int>
    {
        std::pair<int, int> p;
        SDL_GetWindowPosition(window.get(), &p.first, &p.second);
        return p;
    }

    auto Window::getSize() -> std::pair<int, int>
    {
        std::pair<int, int> p;
        SDL_GetWindowSize(window.get(), &p.first, &p.second);
        return p;
    }

    auto Window::getID() -> Uint32
    {
        return SDL_GetWindowID(window.get());
    }

    auto Window::subWindow(float x, float y, float w, float h) -> Window &
    {
        x = std::clamp(x, -1.0f, 1.0f);
        y = std::clamp(y, -1.0f, 1.0f);
        w = std::clamp(w, 0.0f, 1.0f);
        h = std::clamp(h, 0.0f, 1.0f);

        auto pair = getPosition();
        auto size = getSize();
        return subWindows.emplace_back(
            static_cast<float>(pair.first + size.first) * (x + 1.0f) / 2.0f, static_cast<float>(pair.second + size.second) * (y + 1.0f) / 2.0f,
            static_cast<float>(size.first) * w, static_cast<float>(size.second) * h,
            SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_BORDERLESS);
    }

    void Window::clean()
    {
        glClear(GL_COLOR_BUFFER_BIT);
    }

    void Window::update()
    {
    }

    void Window::render()
    {
        if (minimised)
        {
            return;
        }

        clean();

        // auto pair = getSize();

        // glRasterPos2i(0, 0);
        // glBindBuffer(GL_PIXEL_UNPACK_BUFFER, glPixelBuffer);
        // glDrawPixels(pair.first, pair.second, GL_RGBA, GL_UNSIGNED_BYTE, 0);

        // glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

        // glRasterPos2i(0, 0);
        for (const auto &rec : rectangles)
        {
            rec.render();
        }

        SDL_GL_SwapWindow(window.get());
    }

    void Window::redraw()
    {
        if (minimised)
        {
            return;
        }

        if (glPixelBuffer)
        {
            glDeleteBuffers(1, &glPixelBuffer);
        }

        auto pair = getSize();

        // glGenBuffers(1, &glPixelBuffer);
        // glBindBuffer(GL_PIXEL_UNPACK_BUFFER, glPixelBuffer);
        // glBufferData(GL_PIXEL_UNPACK_BUFFER, pair.first * pair.second * sizeof(GLubyte) * 4, 0, GL_STREAM_DRAW);
        // glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

        glViewport(0, 0, pair.first, pair.second);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        // glMatrixMode(GL_PROJECTION);

        // double rad = std::numbers::pi/180.0;
        // double perspective[16] = {
        //     1.0/(aspect*std::tan(60.0*rad)), 0, 0, 0,
        //     0, 1.0/(std::tan(60.0*rad)), 0, 0,
        //     0, 0, -()
        // };

        // glLoadMatrixd()
        if (pair.first > pair.second)
        {
            double aspect = static_cast<double>(pair.first) / static_cast<double>(pair.second);
            glOrtho(-1.0 * aspect, 1.0 * aspect, -1.0, 1.0, 0.0, 1.0);
        }
        else
        {
            double aspect = static_cast<double>(pair.second) / static_cast<double>(pair.first);
            glOrtho(-1.0, 1.0, -1.0 * aspect, 1.0 * aspect, 0.0, 1.0);
        }
    }

    Rectangle &Window::addRectangle(float x, float y, float w, float h)
    {
        // auto pair = getPosition();
        return rectangles.emplace_back(std::clamp(x, -1.0f, 1.0f), std::clamp(y, -1.0f, 1.0f), std::clamp(w, 0.0f, 1.0f), std::clamp(h, 0.0f, 1.0f));
    }

    void Window::setActive()
    {
        SDL_GL_MakeCurrent(window.get(), glContext);
    }

    void Window::addCallback(Uint32 e, std::function<void(const SDL_Event &)> fun)
    {
        events.insert_or_assign(e, fun);
    }

    void Window::process(const SDL_Event &e)
    {
        auto itr = events.find(e.type);
        if (itr != events.end())
        {
            itr->second(e);
        }
    }

    void Window::processWindowEvent(const SDL_Event &e)
    {
        switch (e.window.event)
        {
        case SDL_WINDOWEVENT_MINIMIZED:
            minimised = true;
            break;
        case SDL_WINDOWEVENT_MAXIMIZED:
            break;
        case SDL_WINDOWEVENT_RESTORED:
            minimised = false;
            break;
        case SDL_WINDOWEVENT_SIZE_CHANGED:
            redraw();
            for (auto &w : subWindows)
            {
                w.redraw();
            }
            break;
        case SDL_WINDOWEVENT_MOVED:
            for (auto &w : subWindows)
            {
                // window.getPosition();
                SDL_SetWindowPosition(w.window.get(), e.window.data1, e.window.data2);
                SDL_RaiseWindow(w.window.get());
            }
            SDL_SetWindowInputFocus(window.get());
            break;
        default:
            break;
        }
    }

    void Window::mouseScroll(const SDL_Event &e)
    {
        auto size = getSize();

        int mx = 0, my = 0;
        SDL_GetMouseState(&mx, &my);

        for (auto &r : rectangles)
        {
            if (static_cast<int>((r.x + 1.0f) / 2.0f * static_cast<float>(size.first)) < mx &&
                static_cast<int>((r.x + r.w + 1.0f) / 2.0f * static_cast<float>(size.first)) > mx &&
                size.second - static_cast<int>((r.y + 1.0f) / 2.0f * static_cast<float>(size.second)) > my &&
                size.second - static_cast<int>((r.y + r.h + 1.0f) / 2.0f * static_cast<float>(size.second)) < my)
            {
                r.process(e);
            }
        }
    }

    void Window::mouseClick(const SDL_Event &e)
    {
        auto size = getSize();

        for (auto &r : rectangles)
        {

            if (static_cast<int>((r.x + 1.0f) / 2.0f * static_cast<float>(size.first)) < e.button.x &&
                static_cast<int>((r.x + r.w + 1.0f) / 2.0f * static_cast<float>(size.first)) > e.button.x &&
                size.second - static_cast<int>((r.y + 1.0f) / 2.0f * static_cast<float>(size.second)) > e.button.y &&
                size.second - static_cast<int>((r.y + r.h + 1.0f) / 2.0f * static_cast<float>(size.second)) < e.button.y)
            {
                if (e.button.type == SDL_MOUSEBUTTONUP && dragObject.has_value())
                {
                    r.process(dragObject.value());
                }
                else
                {
                    r.process(e);
                }
            }

            if (e.button.type == SDL_MOUSEBUTTONUP)
            {
                r.process(e);
            }
        }
        
        if (e.button.type == SDL_MOUSEBUTTONUP)
        {
            dragObject.reset();
        }
    }

    void Window::mouseMotion(const SDL_Event &e)
    {
        for (auto &r : rectangles)
        {
            r.process(e);
        }
    }

    void Window::userDrop(const SDL_Event &e)
    {
        // auto size = getSize();

        // int mx = 0, my = 0;
        // SDL_GetMouseState(&mx, &my);

        // for (auto &r : rectangles)
        // {
        //     if (r.x < *static_cast<float *>(e.user.data1) &&
        //         r.x + 2.0f * r.w > *static_cast<float *>(e.user.data1) &&
        //         r.y < *static_cast<float *>(e.user.data2) &&
        //         r.y + 2.0f * r.h > *static_cast<float *>(e.user.data2))
        //     {
        //         r.process(e);
        //     }
        // }

        dragObject.emplace(e);
    }

} // namespace gui
