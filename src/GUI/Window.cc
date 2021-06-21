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

        addCallback(SDL_WINDOWEVENT, std::bind(Window::windowEvent, this, std::placeholders::_1));
        addCallback(SDL_MOUSEWHEEL, std::bind(Window::scrollEvent, this, std::placeholders::_1));
        addCallback(SDL_MOUSEBUTTONUP, std::bind(Window::dragStopEvent, this, std::placeholders::_1));
        addCallback(SDL_MOUSEBUTTONDOWN, std::bind(Window::dragStartEvent, this, std::placeholders::_1));
        addCallback(SDL_MOUSEMOTION, std::bind(Window::dragEvent, this, std::placeholders::_1));
        addCallback(Rectangle::dropEventData, std::bind(Window::userDrop, this, std::placeholders::_1));
        addCallback(Rectangle::volumeEventData, std::bind(Window::userDrop, this, std::placeholders::_1));

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

        glViewport(0, 0, pair.first, pair.second);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();

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
        return rectangles.emplace_back(std::clamp(x, -1.0f, 1.0f), std::clamp(y, -1.0f, 1.0f), std::clamp(w, 0.0f, 1.0f), std::clamp(h, 0.0f, 1.0f));
    }

    void Window::setActive()
    {
        SDL_GL_MakeCurrent(window.get(), glContext);
    }

    void Window::windowEvent(const SDL_Event &e)
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

    void Window::scrollEvent(const SDL_Event &e)
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
                break;
            }
        }
    }

    void Window::dragStartEvent(const SDL_Event &e)
    {
        auto size = getSize();

        for (auto &r : rectangles)
        {
            if (static_cast<int>((r.x + 1.0f) / 2.0f * static_cast<float>(size.first)) < e.button.x &&
                static_cast<int>((r.x + r.w + 1.0f) / 2.0f * static_cast<float>(size.first)) > e.button.x &&
                size.second - static_cast<int>((r.y + 1.0f) / 2.0f * static_cast<float>(size.second)) > e.button.y &&
                size.second - static_cast<int>((r.y + r.h + 1.0f) / 2.0f * static_cast<float>(size.second)) < e.button.y)
            {
                r.process(e);
                break;
            }
        }
    }

    void Window::dragStopEvent(const SDL_Event &e)
    {
        auto size = getSize();

        if (dragObject.has_value())
        {
            if (dragObject->user.type == gui::Rectangle::dropEventData)
            {
                for (auto &r : rectangles)
                {

                    if (static_cast<int>((r.x + 1.0f) / 2.0f * static_cast<float>(size.first)) < e.button.x &&
                        static_cast<int>((r.x + r.w + 1.0f) / 2.0f * static_cast<float>(size.first)) > e.button.x &&
                        size.second - static_cast<int>((r.y + 1.0f) / 2.0f * static_cast<float>(size.second)) > e.button.y &&
                        size.second - static_cast<int>((r.y + r.h + 1.0f) / 2.0f * static_cast<float>(size.second)) < e.button.y)
                    {
                        r.process(dragObject.value());
                        break;
                    }
                }
            }

            static_cast<Rectangle *>(dragObject->user.data1)->process(e);
            dragObject.reset();
        }

    }

    void Window::dragEvent(const SDL_Event &e)
    {
        for (auto &r : rectangles)
        {
            r.process(e);
        }
    }

    void Window::userDrop(const SDL_Event &e)
    {
        dragObject.emplace(e);
    }

} // namespace gui
