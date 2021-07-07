#ifndef GUI_SDL2_WINDOW_HH
#define GUI_SDL2_WINDOW_HH

#include <memory>
#include <utility>
#include <functional>
#include <vector>

#include <SDL2/SDL.h>
#include <CL/cl2.hpp>
#include <GL/glew.h>
#include <GL/gl.h>
#include <glm/glm.hpp>


#include "Rectangle.hh"

#include "../Events/EventManager.hh"

namespace gui
{

    class Window : public Rectangle, public events::EventManager<Window>
    {
    private:
        static std::once_flag glInit;
        std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)> window;
        // std::unique_ptr<SDL_Surface> surface;
        SDL_GLContext glContext;

        std::optional<SDL_Event> dragObject;

        GLuint vShader = 0, fShader = 0, program = 0;

        void initGL();

        void windowEvent(const SDL_Event &);
        void dragStartEvent(const SDL_Event &e);
        void dragStopEvent(const SDL_Event &e);
        void dragEvent(const SDL_Event &e);
        void userDrop(const SDL_Event &e);

        bool minimised = false;

        GLint projectionUni;

        glm::mat4 projection;

    public:

        using events::EventManager<Window>::addCallback;

        Window(unsigned int w = 640, unsigned int h = 480, Uint32 flags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
        Window(unsigned int x, unsigned int y, unsigned int w, unsigned int h, Uint32 flags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
        ~Window();

        Window(const Window &) = delete;
        Window(Window &&) = default;

        void clean();
        void update();
        void render();
        void renderArrow();
        void redraw();
        void setActive();
        auto getPosition() -> std::pair<int, int>;
        auto getSize() -> std::pair<int, int>;
        auto getID() -> Uint32;

        void process(const SDL_Event &e);
    };

} // namespace gui

#endif