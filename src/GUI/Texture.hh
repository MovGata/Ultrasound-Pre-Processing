#ifndef GUI_TEXTURE_HH
#define GUI_TEXTURE_HH

#include <string>

#include <GL/glew.h>
#include <GL/gl.h>
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_ttf.h>

namespace gui
{

    class Texture
    {
    private:
        GLuint texture;
        GLsizei textureW = 0, textureH = 0;

    public:
        Texture(unsigned int w = 1, unsigned int h = 1);
        ~Texture();

        Texture(const Texture &) = delete;

        void update(GLuint pixelBuffer);
        void addText(TTF_Font *f, const std::string &str);
        void fill(SDL_Colour c);

        operator GLuint();
    };

} // namespace gui

#endif