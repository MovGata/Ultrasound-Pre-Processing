#include <cmath>
#include <fstream>
#include <iostream>
#include <numbers>

#include <gl/glew.h>
#include <gl/gl.h>
#include <gl/glext.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Window.hh"

namespace gui
{

    std::once_flag Window::glInit;

    Window::Window(unsigned int www, unsigned int hhh, Uint32 flags) : Window(SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, www, hhh, flags) {}
    Window::Window(unsigned int xx, unsigned int yy, unsigned int www, unsigned int hhh, Uint32 flags) : window(nullptr, SDL_DestroyWindow), projection(1.0f)
    {

        window.reset(SDL_CreateWindow("Ultrasound Pre-Processor", xx, yy, www, hhh, flags));
        if (window == nullptr)
        {
            std::cout << "SDL2 window initialisation failed, error: " << SDL_GetError() << std::endl;
            return;
        }

        addCallback(SDL_WINDOWEVENT, Window::windowEvent);
        addCallback(SDL_MOUSEWHEEL, Window::scrollEvent);
        addCallback(SDL_MOUSEBUTTONUP, Window::dragStopEvent);
        addCallback(SDL_MOUSEBUTTONDOWN, Window::dragStartEvent);
        addCallback(SDL_MOUSEMOTION, Window::dragEvent);
        addCallback(Rectangle::dropEventData, Window::userDrop);
        addCallback(Rectangle::volumeEventData, Window::userDrop);
        addCallback(Rectangle::moveEventData, Window::userDrop);

        glContext = SDL_GL_CreateContext(window.get());
        if (glContext == nullptr)
        {
            std::cout << "OpenGL context initialisation failed, error: " << SDL_GetError() << std::endl;
            window.release();
            return;
        }

        modelview = glm::mat4(1.0f);

        std::call_once(
            glInit,
            [this]
            {
                GLenum gError = glewInit();
                if (gError != GLEW_OK)
                {
                    std::cout << "Glew init error: " << glewGetErrorString(gError) << std::endl;
                    window.release();
                    return;
                }

                if (!GLEW_VERSION_3_3)
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

                initGL();

                glDisable(GL_CULL_FACE);

                // glEnable(GL_TEXTURE_2D);
                glDisable(GL_DEPTH_TEST);

                arrow = Rectangle(0.5f, 0.0f, 0.5f, 0.005f);
                arrow.setBG({0xFF, 0xFF, 0xFF, 0xFF});
                arrow.allocTexture(1, 1);
                arrow.update(modelview);
            });


        clean();
        redraw();
    }

    Window::~Window()
    {
        glUseProgram(0);
        glDetachShader(program, fShader);
        glDetachShader(program, vShader);
        glDeleteShader(fShader);
        glDeleteShader(vShader);
        glDeleteProgram(program);
        SDL_GL_DeleteContext(glContext);
    }

    void Window::initGL()
    {
        const char *vSource[] =
            {
#include "../../shaders/vertex.glsl"
            };
        const char *fSource[] =
            {
#include "../../shaders/fragment.glsl"
            };

        program = glCreateProgram();
        vShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vShader, 1, vSource, NULL);
        glCompileShader(vShader);
        GLint res = GL_FALSE;
        glGetShaderiv(vShader, GL_COMPILE_STATUS, &res);
        if (res != GL_TRUE)
        {
            std::cerr << "Unable to compile vertex shader " << vShader << ".\n";
            int len = 0;
            glGetShaderiv(vShader, GL_INFO_LOG_LENGTH, &len);
            std::string log;
            log.resize(len);
            glGetShaderInfoLog(vShader, len, &len, log.data());
            if (len)
            {
                std::fstream fout;
                fout.open(std::string("./build/logs/") + "vertex.glsl" + std::string(".txt"), std::fstream::out);
                fout << log;
            }
            return;
        }

        glAttachShader(program, vShader);

        fShader = glCreateShader(GL_FRAGMENT_SHADER);

        glShaderSource(fShader, 1, fSource, NULL);
        glCompileShader(fShader);
        res = GL_FALSE;
        glGetShaderiv(fShader, GL_COMPILE_STATUS, &res);
        if (res != GL_TRUE)
        {
            std::cerr << "Unable to compile fragment shader " << fShader << ".\n";
            int len = 0;
            glGetShaderiv(fShader, GL_INFO_LOG_LENGTH, &len);
            std::string log;
            log.resize(len);
            glGetShaderInfoLog(fShader, len, &len, log.data());
            if (len)
            {
                std::fstream fout;
                fout.open(std::string("./build/logs/") + "fragment.glsl" + std::string(".txt"), std::fstream::out);
                fout << log;
            }
            return;
        }

        glAttachShader(program, fShader);

        glBindAttribLocation(program, 0, "position");
        glBindAttribLocation(program, 1, "texcoord");

        glLinkProgram(program);
        res = GL_FALSE;
        glGetProgramiv(program, GL_LINK_STATUS, &res);
        if (res != GL_TRUE)
        {
            std::cerr << "Unable to compile program " << fShader << ".\n";
            int len = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &len);
            std::string log;
            log.resize(len);
            glGetProgramInfoLog(program, len, &len, log.data());
            if (len)
            {
                std::cerr << log << '\n';
            }
            return;
        }

        glUseProgram(program);

        projectionUni = glGetUniformLocation(program, "projection");
        modelviewUni = glGetUniformLocation(program, "modelview");
    }

    void Window::process(const SDL_Event &e)
    {
        EventManager<Window>::process(this, e);
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

        for (const auto &rec : subRectangles)
        {
            rec->render();
        }

        for (const auto &rec : subRectangles)
        {
            rec->renderChildren();
        }

        for (const auto &rec : subRectangles)
        {
            rec->renderLinks();
        }

        renderArrow();

        SDL_GL_SwapWindow(window.get());
    }

    void Window::renderArrow()
    {
        if (minimised)
        {
            return;
        }

        if (Rectangle::mouseArrow.has_value())
        {
            arrow.tf = *mouseArrow;
            Rectangle::arrow.render();
        }
    }

    void Window::redraw()
    {
        if (minimised)
        {
            return;
        }

        auto pair = getSize();

        glViewport(0, 0, pair.first, pair.second);

        if (pair.first > pair.second)
        {
            float aspect = static_cast<float>(pair.first) / static_cast<float>(pair.second);
            x = -1.0f;
            y = -1.0f / aspect;
            w = 2.0f;
            h = 2.0f * std::abs(y);
        }
        else
        {
            float aspect = static_cast<float>(pair.second) / static_cast<float>(pair.first);
            x = -1.0f / aspect;
            y = -1.0f;
            w = 2.0f * std::abs(x);
            h = 2.0f;
        }

        projection = glm::ortho(x, x + w, y, y + h, 0.0f, 1.0f);

        glUniformMatrix4fv(projectionUni, 1, GL_FALSE, reinterpret_cast<const GLfloat *>(&projection[0]));
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
            break;
        case SDL_WINDOWEVENT_MOVED:
            break;
        default:
            break;
        }
    }

    void Window::dragStartEvent(const SDL_Event &e)
    {
        if (dragObject.has_value())
        {
            Rectangle *ptr = static_cast<Rectangle *>(dragObject->user.data2);
            ptr->process(*dragObject);
        }
        else
        {
            std::pair<int, int> size;
            SDL_GetWindowSize(SDL_GetWindowFromID(e.button.windowID), &size.first, &size.second);

            int mx = e.button.x, my = e.button.y;

            glm::vec4 mp = {0.0f, 0.0f, 0.0f, 1.0f};

            mp.x = std::lerp(x, x + w, static_cast<float>(mx) / static_cast<float>(size.first));
            mp.y = std::lerp(y, y + h, 1.0f - static_cast<float>(my) / static_cast<float>(size.second));

            mp = modelview * mp;

            for (auto &r : subRectangles)
            {
                if (r->contains(mp.x, mp.y))
                {
                    r->process(e);
                    break;
                }
            }
        }

    }

    void Window::dragStopEvent(const SDL_Event &e)
    {

        if (dragObject.has_value())
        {
            std::shared_ptr<Rectangle> rec = static_cast<std::weak_ptr<Rectangle> *>(dragObject->user.data1)->lock();
            if (dragObject->user.type == gui::Rectangle::dropEventData)
            {
                for (auto &r : subRectangles)
                {
                    if (r->containsTF(*rec))
                    {
                        r->process(dragObject.value());
                        break;
                    }
                }
            }
            else if (dragObject->user.type == gui::Rectangle::moveEventData)
            {
                for (auto &r : subRectangles)
                {
                    if (r->contains(*rec))
                    {
                        r->process(dragObject.value());
                        break;
                    }
                }
            }

            static_cast<std::weak_ptr<Rectangle> *>(dragObject->user.data1)->lock()->process(e);
            dragObject.reset();
        }
    }

    void Window::dragEvent(const SDL_Event &e)
    {
        if (dragObject.has_value())
        {
            static_cast<std::weak_ptr<Rectangle> *>(dragObject->user.data1)->lock()->process(e);
        }
    }

    void Window::userDrop(const SDL_Event &e)
    {
        dragObject.emplace(e);
    }

} // namespace gui
