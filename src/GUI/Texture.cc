#include "Texture.hh"

#include <iostream>
#include <vector>

namespace gui
{

    Texture::Texture(unsigned int w, unsigned int h) : textureW(w), textureH(h)
    {

        std::vector<uint32_t> clr(w * h);
        // clr.reserve(wp * hp);
        // std::fill_n(std::back_inserter(clr), wp * hp, static_cast<uint32_t>(colour.r) << 24 | static_cast<uint32_t>(colour.g) << 16 | static_cast<uint32_t>(colour.b) << 8 | static_cast<uint32_t>(colour.a));

        glGenTextures(1, &texture);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, clr.data());
        glBindTexture(GL_TEXTURE_2D, 0);

        GLenum err;
        if ((err = glGetError()))
            std::cout << "Texture err: " << err << std::endl;
    }

    Texture::~Texture()
    {
        glDeleteTextures(1, &texture);
    }

    void Texture::fill(SDL_Colour c)
    {
        std::vector<uint32_t> v;
        v.reserve(textureW * textureH);
        std::fill_n(std::back_inserter(v), textureW * textureH, static_cast<uint32_t>(c.r) << 24 | static_cast<uint32_t>(c.g) << 16 | static_cast<uint32_t>(c.b) << 8 | static_cast<uint32_t>(c.a));

        glBindTexture(GL_TEXTURE_2D, texture);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, textureW, textureH, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, v.data());
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    void Texture::addText(TTF_Font *font, const std::string &str)
    {
        SDL_Surface *s = TTF_RenderText_Blended(font, str.c_str(), SDL_Colour{0xFF, 0xFF, 0xFF, 0xFF});
        if (!s)
        {
            std::cerr << "Failed to create text: " << TTF_GetError() << '\n';
        }
        SDL_Surface *textT = SDL_ConvertSurfaceFormat(s, SDL_PixelFormatEnum::SDL_PIXELFORMAT_RGBA8888, 0);
        SDL_FreeSurface(s);

        glBindTexture(GL_TEXTURE_2D, texture);
        if ((textT->w+1) * (textT->h+1) < textureW * textureH)
        {

            std::vector<uint32_t> clr(textureW * textureH);
            // clr.reserve(textureW * textureH);
            // std::fill_n(std::back_inserter(clr), textureW * textureH, 0);
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, textureW, textureH, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, clr.data());
            glTexSubImage2D(GL_TEXTURE_2D, 0, 1, 1, textT->w, textT->h, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, textT->pixels);
            // for (unsigned int y = 1; y < 1+textT->h; ++y)
            // {
            //     glTexSubImage2D(GL_TEXTURE_2D, 0, 1, y, textT->w, textT->h, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, textT->pixels);
            //     glTexSubImage2D(GL_TEXTURE_2D, 0, 1+text->w, y, textT->w, textT->h, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, textT->pixels);
            // }
            // glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, textureW, textureH, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, clr.data());
        }
        else
        {
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, textureW, textureH, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8, textT->pixels);
        }

        glBindTexture(GL_TEXTURE_2D, 0);
        // text = str;

        SDL_FreeSurface(textT);
    }

    void Texture::update(GLuint pixelBuffer)
    {
        glBindTexture(GL_TEXTURE_2D, texture);

        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pixelBuffer);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, textureW, textureH, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, 0);
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

        glBindTexture(GL_TEXTURE_2D, 0);
    }

    Texture::operator GLuint()
    {
        return texture;
    }

} // namespace gui
