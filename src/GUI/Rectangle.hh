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
#include "../Events/EventManager.hh"

namespace gui
{

    class Rectangle : public events::EventManager
    {
    private:
        std::optional<Volume> volume;
        std::string text;
        std::unordered_map<Uint32, std::function<void(const SDL_Event &)>> events;
        // std::unique_ptr<SDL_Surface, decltype(&SDL_FreeSurface)> text;
        SDL_Colour colour = {0x4F, 0x4F, 0x4F, 0xFF};

        TTF_Font *font;

        std::vector<Rectangle> subRectangles;        

        void dragEvent(const SDL_Event &e);
        void dragStopEvent(const SDL_Event &e);

        void volumeBypass(const SDL_Event &e);
        void volumeStopEvent(const SDL_Event &e);

    public:
        static const Uint32 dropEventData;
        static const Uint32 volumeEventData;

        GLuint texture = 0;
        GLuint pixelBuffer = 0;
        GLsizei ww = 0, hh = 0;
        float x, y, w, h, offX = 0, offY = 0;

        Rectangle(float x, float y, float w, float h);
        ~Rectangle();

        Rectangle(const Rectangle &) = delete;
        Rectangle(Rectangle &&) = default;

        Volume &allocVolume(unsigned int depth, unsigned int length, int unsigned width, const std::vector<uint8_t> &data);
        void allocTexture(unsigned int w, unsigned int h);
        void addText(TTF_Font *f, const std::string &str);
        void setBG(SDL_Colour c);
        void render() const;

        void volumeStartEvent(const SDL_Event &e);
        void dragStartEvent(const SDL_Event &e);
        void dropEvent(const SDL_Event &e);
    };

} // namespace gui

#endif