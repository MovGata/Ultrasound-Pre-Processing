#include <iostream>

#include <gl/glew.h>
#include <gl/gl.h>
#include <gl/glext.h>

#include "Window.hh"

namespace gui
{

    Window::Window(unsigned int w, unsigned int h) : window(nullptr, SDL_DestroyWindow), width(w), height(h)
    {

        window.reset(SDL_CreateWindow("Ultrasound Pre-Processor", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN));
        if (window == nullptr)
        {
            std::cout << "SDL2 window initialisation failed, error: " << SDL_GetError() << std::endl;
            return;
        }

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

    void Window::clean()
    {
        glClear(GL_COLOR_BUFFER_BIT);
    }

    void Window::update()
    {
        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT)
            {
                quit = true;
            }
        }
    }

    void Window::render()
    {
        clean();
        glRasterPos2i(0, 0);

        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, glPixelBuffer);
        glDrawPixels(width, height, GL_RGBA, GL_UNSIGNED_BYTE, 0);
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
        SDL_GL_SwapWindow(window.get());
    }

    void Window::redraw()
    {
        if (glPixelBuffer)
        {
            glDeleteBuffers(1, &glPixelBuffer);
        }

        glGenBuffers(1, &glPixelBuffer);
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, glPixelBuffer);
        glBufferData(GL_PIXEL_UNPACK_BUFFER, width * height * sizeof(GLubyte) * 4, 0, GL_STREAM_DRAW);
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

        glViewport(0, 0, width, height);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0.0, 1.0, 0.0, 1.0, 0.0, 1.0);
    }

} // namespace gui
