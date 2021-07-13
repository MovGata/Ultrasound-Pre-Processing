#include "Instance.hh"

#include <fstream>
#include <iostream>

#include <gl/glew.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_opengl.h>
#include <gl/glu.h>

namespace gui
{
    std::once_flag Instance::onceFlag;

    GLint Instance::projectionUni = -1;
    GLint Instance::modelviewUni = -1;

    Instance::Instance() : font(nullptr, TTF_CloseFont)
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
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    }

    Instance::~Instance()
    {
        glUseProgram(0);
        glDetachShader(program, fShader);
        glDetachShader(program, vShader);
        glDeleteShader(fShader);
        glDeleteShader(vShader);
        glDeleteProgram(program);
        SDL_Quit();
    }

    void Instance::initGL()
    {
        std::call_once(
            onceFlag,
            [this]
            {
                GLenum gError = glewInit();
                if (gError != GLEW_OK)
                {
                    std::cout << "Glew init error: " << glewGetErrorString(gError) << std::endl;
                    return;
                }

                if (!GLEW_VERSION_3_3)
                {
                    std::cout << "OpenGL 2.1 minimum" << std::endl;
                    return;
                }

                // Vsync
                if (SDL_GL_SetSwapInterval(1) < 0)
                {
                    std::cout << "VSync unavailable: " << SDL_GetError() << std::endl;
                }

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

                glDisable(GL_CULL_FACE);

                // glEnable(GL_TEXTURE_2D);
                glDisable(GL_DEPTH_TEST);
                glClearColor(0.117f, 0.117f, 0.117f, 1.0f);
            });
    }

    TTF_Font *Instance::loadFont(const std::string &url)
    {
        font.reset(TTF_OpenFont(url.c_str(), 16));
        return font.get();
    }

} // namespace gui
