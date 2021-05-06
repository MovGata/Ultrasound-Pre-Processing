#include <iostream>

#include <SDL2/SDL.h>

#include "Instance.hh"


namespace gui
{
    
    Instance::Instance(/* args */)
    {
        if (SDL_Init(SDL_INIT_VIDEO) < 0)
        {
            std::cout << "SDL2 initialisation failed, error: " << SDL_GetError() << std::endl;
        }
    }
    
    Instance::~Instance()
    {
        SDL_Quit();
    }
    
} // namespace gui
