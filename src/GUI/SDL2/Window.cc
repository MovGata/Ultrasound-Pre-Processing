#include <iostream>

#include "Window.hh"

namespace gui
{
    
    Window::Window(unsigned int w, unsigned int h) : window(nullptr, SDL_DestroyWindow), width(w), height(h)
    {
        
        
        window.reset(SDL_CreateWindow("Ultrasound Pre-Processor", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_SHOWN));
        if (window == nullptr)
        {
            std::cout << "SDL2 window initialisation failed, error: " << SDL_GetError() << std::endl;
            return;
        }

        surface.reset(SDL_GetWindowSurface(window.get()));

        clean();
    }
    
    Window::~Window()
    {
    }
    
    void Window::clean()
    {
        SDL_FillRect(surface.get(), NULL, SDL_MapRGB(surface->format, 0xEF, 0xFF, 0xB3));
    }

    void Window::update()
    {
        SDL_UpdateWindowSurface(window.get());
    }
    
} // namespace gui
