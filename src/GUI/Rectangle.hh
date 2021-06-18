#ifndef GUI_RECTANGLE_HH
#define GUI_RECTANGLE_HH

#include <GL/glew.h>
#include <GL/gl.h>

#include <unordered_map>
#include <functional>
#include <memory>
#include <optional>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "../Data/Volume.hh"

namespace gui
{

    class Rectangle
    {
    private:
        std::optional<Volume> volume;
        std::unordered_map<Uint32, std::function<void(const SDL_Event &)>> events;
        std::unique_ptr<SDL_Surface, decltype(&SDL_FreeSurface)> text;
        bool mouseDown = false;
        SDL_Colour colour = {0x4F, 0x4F, 0x4F, 0xFF};

    public:
        GLuint texture = 0;
        GLuint pixelBuffer = 0;
        GLsizei ww = 0, hh = 0;
        float x, y, w, h;

        Rectangle(float x, float y, float w, float h);
        ~Rectangle();

        Rectangle(const Rectangle &) = delete;
        Rectangle(Rectangle &&) = default;

        Volume &allocVolume(unsigned int depth, unsigned int length, int unsigned width, const std::vector<uint8_t> &data);
        void allocTexture(unsigned int w, unsigned int h);
        void addText(TTF_Font &font, const std::string &str);
        void setBG(SDL_Colour c);
        void render() const;

        void addCallback(Uint32 e, std::function<void(const SDL_Event &)> fun);
        void process(const SDL_Event &e);

        void zoom(const SDL_Event &e);
        void mouseMotion(const SDL_Event &e);
    };

} // namespace gui

#endif