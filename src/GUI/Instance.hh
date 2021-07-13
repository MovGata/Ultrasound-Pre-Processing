#ifndef GUI_SDL2_INSTANCE_HH
#define GUI_SDL2_INSTANCE_HH

#include <memory>
#include <mutex>

#include <GL/glew.h>
#include <GL/gl.h>

#include <SDL2/SDL_ttf.h>

namespace gui
{
    
    class Instance
    {
    private:
        static std::once_flag onceFlag;
        std::unique_ptr<TTF_Font, decltype(&TTF_CloseFont)> font;
        GLuint vShader = 0, fShader = 0, program = 0;


    public:
        Instance();
        ~Instance();

        void initGL();

        static GLint projectionUni;
        static GLint modelviewUni;

        TTF_Font *loadFont(const std::string &url);
    };

} // namespace gui

#endif