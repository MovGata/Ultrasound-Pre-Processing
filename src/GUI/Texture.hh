#ifndef GUI_TEXTURE_HH
#define GUI_TEXTURE_HH

#include <string>
#include <vector>

#include <GL/glew.h>
#include <GL/gl.h>
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_ttf.h>
#include <CL/cl2.hpp>

namespace gui
{

    class Texture
    {
    private:
        GLuint texture;


    public:
        GLsizei textureW = 0, textureH = 0;
        static TTF_Font *lastFont;
        
        Texture(unsigned int w = 1, unsigned int h = 1);
        ~Texture();

        Texture(const Texture &);

        void update(GLuint pixelBuffer);
        void addText(TTF_Font *f, const std::string &str);
        void addText(const std::string &str);
        void fill(SDL_Colour c);

        void rotate(unsigned int x, unsigned int y, unsigned int w, unsigned int h);

        operator GLuint();
    };

} // namespace gui

#endif