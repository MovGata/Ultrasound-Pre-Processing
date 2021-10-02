#ifndef GUI_SDL2_WINDOW_HH
#define GUI_SDL2_WINDOW_HH

#include <memory>
#include <utility>
#include <functional>
#include <iostream>
#include <ranges>
#include <utility>
#include <variant>
#include <vector>

#include <SDL2/SDL.h>
#include <CL/cl2.hpp>
#include <GL/glew.h>
#include <GL/gl.h>
#include <glm/glm.hpp>

#include "Instance.hh"
#include "Rectangle.hh"
#include "Kernel.hh"
#include "Renderer.hh"

#include "../Events/Concepts.hh"
#include "../Events/EventManager.hh"

namespace gui
{
    class Window
    {
    private:
        std::shared_ptr<events::EventManager> eventManager;
        std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)> window;
        SDL_GLContext glContext;
        bool minimised = false;
        glm::mat4 projection;
        std::pair<float, float> size;

        void windowEvent(const SDL_Event &e);

    public:
        std::vector<std::shared_ptr<gui::Rectangle>> drawables;
        std::vector<std::shared_ptr<gui::Renderer>> renderers;
        std::shared_ptr<gui::Kernel> kernel;

        Window(unsigned int width = 640, unsigned int height = 480, Uint32 flags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
        Window(unsigned int xPos, unsigned int yPos, unsigned int width, unsigned int height, Uint32 flags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);

        ~Window();

        Window(const Window &) = delete;
        Window(Window &&) = default;

        void clean();
        void update();
        void draw();
        void redraw();
        void redraw(const SDL_Event &e);
        auto getPosition() -> std::pair<int, int>;
        auto getSize() -> std::pair<int, int>;
        auto getID() -> Uint32;
        void eraseDrawable(std::size_t i);
        void process(const SDL_Event &e);
        void addDrawable(std::shared_ptr<gui::Rectangle> &&d);
    };

} // namespace gui

#endif