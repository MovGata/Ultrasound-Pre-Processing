#ifndef GUI_SDL2_WINDOW_HH
#define GUI_SDL2_WINDOW_HH

#include <array>
#include <memory>

#include <SDL2/SDL.h>
#include <CL/cl2.hpp>
#include <gl/gl.h>

namespace gui
{

class Window
{
private:
    std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)> window;
    // std::unique_ptr<SDL_Surface> surface;
    SDL_GLContext glContext;
    float rotation = 0.0f;

    SDL_Event e;

public:
    std::array<float, 12> invMVTransposed;
    GLuint glPixelBuffer;
    bool quit = false;

    Window(unsigned int w= 640, unsigned int h = 480);
    ~Window();

    unsigned int width;
    unsigned int height;

    void clean();
    void update();
    void render();
    void redraw();
};

} // namespace gui

#endif