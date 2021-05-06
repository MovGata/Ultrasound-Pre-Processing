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
    }
    
    Window::~Window()
    {
    }
    
    
} // namespace gui
