#ifndef GUI_RECTANGLE_HH
#define GUI_RECTANGLE_HH

#include <GL/glew.h>
#include <GL/gl.h>
#include <glm/glm.hpp>

#include <unordered_map>
#include <functional>
#include <memory>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "../Data/Volume.hh"
#include "../Events/EventManager.hh"

namespace gui
{

    class Rectangle : public events::EventManager<Rectangle>
    {
    private:
        static std::once_flag onceFlag;
        static GLuint vBuffer, tBuffer, vArray;


        std::shared_ptr<Volume> volume;
        
        SDL_Colour colour = {0x4F, 0x4F, 0x4F, 0xFF};

        TTF_Font *font = nullptr;

        float angle;

        void dragEvent(const SDL_Event &e);
        void dragStopEvent(const SDL_Event &e);

        void moveEvent(const SDL_Event &e);
        void moveStopEvent(const SDL_Event &e);

        void volumeBypass(const SDL_Event &e);
        void volumeStopEvent(const SDL_Event &e);

    protected:
        static Rectangle arrow;
        static std::optional<glm::mat4> mouseArrow;

        mutable bool modified = false;

    public:
        std::string text;
        bool visible = true;
        glm::mat4 modelview;
        glm::mat4 tf;

        static const Uint32 showEventData;
        static const Uint32 dropEventData;
        static const Uint32 moveEventData;
        static const Uint32 volumeEventData;

        static constexpr std::size_t DRAG_FILTER = 0;
        static constexpr std::size_t DRAG_LINK = 1;

        static constexpr std::size_t MOVE_FILTER = 0;

        static constexpr std::size_t VOLUME_ROTATE = 0;
        static constexpr std::size_t VOLUME_DRAG = 1;

        static GLint modelviewUni;

        std::vector<std::shared_ptr<Rectangle>> subRectangles;
        std::weak_ptr<Rectangle> eventRectangle;
        
        std::vector<std::pair<std::weak_ptr<Rectangle>, std::shared_ptr<glm::mat4>>> inlinks;
        std::vector<std::pair<std::weak_ptr<Rectangle>, std::shared_ptr<glm::mat4>>> outlinks;


        std::shared_ptr<GLuint> texture, pixelBuffer;

        GLsizei ww = 0, hh = 0;
        float w, h, x, y;

        Rectangle() = default;
        Rectangle(float x, float y, float w, float h);
        ~Rectangle() = default;

        Rectangle(const Rectangle &) = default;
        Rectangle(Rectangle &&) = default;

        std::weak_ptr<Volume> allocVolume(unsigned int depth, unsigned int length, int unsigned width, const std::vector<uint8_t> &data);
        void allocTexture(unsigned int w, unsigned int h);
        void addText(TTF_Font *f, const std::string &str);
        void setBG(SDL_Colour c);
        void render() const;
        void renderChildren() const;
        void renderLinks();

        void update(const glm::mat4 &mv);

        bool contains(float x, float y) const;
        bool containsTF(float x, float y) const;
        
        bool contains(const Rectangle &rec) const;
        bool containsTF(const Rectangle &rec) const;

        static glm::vec4 globalMouse(Uint32);
        glm::vec4 snapToEdge(const glm::vec4 &) const;

        std::weak_ptr<Rectangle> addRectangle(float x, float y, float w, float h);

        void showEvent(const SDL_Event &e);
        void hideEvent(const SDL_Event &e);

        void volumeStartEvent(const SDL_Event &e);
        void dragStartEvent(const SDL_Event &e);
        void moveStartEvent(const SDL_Event &e);
        void dropEvent(const SDL_Event &e);
        void stopEvent(const SDL_Event &e);
        void linkEvent(const SDL_Event &e);

        void updateArrow(const SDL_Event &e);

        void scaleEvent(const SDL_Event &e);
        void scrollEvent(const SDL_Event &e);

        void doubleDown(const SDL_Event &e);
        void doubleUp(const SDL_Event &e);

        bool process(const SDL_Event &e);

        gui::Rectangle &operator=(const gui::Rectangle &) = default;
        gui::Rectangle &operator=(gui::Rectangle &&) = default;
    };

} // namespace gui

#endif