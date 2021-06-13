#ifndef GUI_SDL2_WINDOW_HH
#define GUI_SDL2_WINDOW_HH

#include <memory>
#include <utility>
#include <functional>

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

        std::vector<Window> subWindows;

        std::unordered_map<Uint32, std::function<void(const SDL_Event &)>> events;

        void processWindowEvent(const SDL_Event &);

        bool minimised = false;

    public:
        GLuint glPixelBuffer;

        Window(unsigned int w = 640, unsigned int h = 480, Uint32 flags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
        Window(unsigned int x, unsigned int y, unsigned int w, unsigned int h, Uint32 flags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
        ~Window();

        Window(const Window &) = delete;
        Window(Window &&) = default;

        auto subWindow(unsigned int x, unsigned int y, unsigned int w, unsigned int h) -> Window &;
        void clean();
        void update();
        void render();
        void redraw();
        void setActive();
        auto getPosition() -> std::pair<int, int>;
        auto getSize() -> std::pair<int, int>;
        auto getID() -> Uint32;

        void addCallback(Uint32 e, std::function<void(const SDL_Event &)> fun);
        void process(const SDL_Event &e);
    };

} // namespace gui

#endif