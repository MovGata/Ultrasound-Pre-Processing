#ifndef GUI_RECTANGLE_HH
#define GUI_RECTANGLE_HH

#include <GL/glew.h>
#include <GL/gl.h>
#include <glm/glm.hpp>

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
        
        SDL_Colour colour = {0x4F, 0x4F, 0x4F, 0xFF};

        TTF_Font *font = nullptr;


        float angle;

        void dragEvent(const SDL_Event &e);
        void dragStopEvent(const SDL_Event &e);

        void volumeBypass(const SDL_Event &e);
        void volumeStopEvent(const SDL_Event &e);

    protected:
        glm::mat4 modelview;

    public:
        static const Uint32 dropEventData;
        static const Uint32 volumeEventData;
        std::vector<Rectangle> subRectangles;

        GLint modelviewUni = -1;

        GLuint vBuffer = 0, tBuffer = 0;
        GLuint vArray = 0;
        GLuint texture = 0;
        GLuint pixelBuffer = 0;
        GLsizei ww = 0, hh = 0;
        float x, y, w, h, offX = 0, offY = 0;

        Rectangle() = default;
        Rectangle(float x, float y, float w, float h);
        ~Rectangle();

        Rectangle(const Rectangle &) = delete;
        Rectangle(Rectangle &&);

        Volume &allocVolume(unsigned int depth, unsigned int length, int unsigned width, const std::vector<uint8_t> &data);
        void allocTexture(unsigned int w, unsigned int h);
        void addText(TTF_Font *f, const std::string &str);
        void setBG(SDL_Colour c);
        void render() const;

        void update(const glm::mat4 &mv);

        bool contains(float x, float y);

        Rectangle &addRectangle(float x, float y, float w, float h);

        void volumeStartEvent(const SDL_Event &e);
        void dragStartEvent(const SDL_Event &e);
        void dropEvent(const SDL_Event &e);

        void scaleEvent(const SDL_Event &e);
        void scrollEvent(const SDL_Event &e);
    };

} // namespace gui

#endif