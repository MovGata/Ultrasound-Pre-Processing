#include <iostream>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_opengl.h>
#include <gl/glu.h>

#include "Instance.hh"

namespace gui
{

    Instance::Instance(/* args */)
    {

        if (SDL_Init(SDL_INIT_VIDEO) < 0)
        {
            std::cout << "SDL2 initialisation failed, error: " << SDL_GetError() << std::endl;
            return;
        }

        if (TTF_Init() < 0)
        {
            std::cout << "TTF initialisation failed, error: " << TTF_GetError() << std::endl;
            return;
        }

        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    }

    Instance::~Instance()
    {
        SDL_Quit();
    }

    TTF_Font *Instance::loadFont(const std::string &url)
    {
        fonts.emplace_back(TTF_OpenFont(url.c_str(), 16), TTF_CloseFont);
        return fonts.back().get();

    }

} // namespace gui
