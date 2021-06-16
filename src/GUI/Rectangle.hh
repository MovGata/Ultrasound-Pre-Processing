#ifndef GUI_RECTANGLE_HH
#define GUI_RECTANGLE_HH

#include <GL/glew.h>
#include <GL/gl.h>

#include <unordered_map>
#include <functional>
#include <optional>

#include <SDL2/SDL.h>

#include "../Data/Volume.hh"

namespace gui
{

    class Rectangle
    {
    private:
        std::optional<Volume> volume;
        std::unordered_map<Uint32, std::function<void(const SDL_Event &)>> events;
        bool mouseDown = false;
    public:
        GLuint texture = 0;
        GLuint pixelBuffer = 0;
        GLsizei ww = 0, hh = 0;
        float x, y, w, h;
        Rectangle(float x, float y, float w, float h);
        ~Rectangle();

        Volume &allocVolume(unsigned int depth, unsigned int length, int unsigned width, const std::vector<uint8_t> &data);
        void allocTexture(unsigned int w, unsigned int h);
        void render() const;

        void addCallback(Uint32 e, std::function<void(const SDL_Event &)> fun);
        void process(const SDL_Event &e);

        void zoom(const SDL_Event &e);
        void mouseMotion(const SDL_Event &e);
    };

} // namespace gui

#endif