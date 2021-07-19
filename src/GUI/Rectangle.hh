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
#include "../Events/GUI.hh"

#include "Texture.hh"

namespace gui
{

    class Rectangle
    {
    private:
        static std::once_flag onceFlag;

        // std::shared_ptr<Volume> volume;

        // SDL_Colour colour = {0x4F, 0x4F, 0x4F, 0xFF};

        // TTF_Font *font = nullptr;


        // void dragEvent(const SDL_Event &e);
        // void dragStopEvent(const SDL_Event &e);

        // void moveEvent(const SDL_Event &e);
        // void moveStopEvent(const SDL_Event &e);

        // void volumeBypass(const SDL_Event &e);
        // void volumeStopEvent(const SDL_Event &e);

    protected:
        // static Rectangle arrow;
        // static std::optional<glm::mat4> mouseArrow;
        mutable bool modified = false;

    public:
        std::string text;
        bool hidden = false;
        glm::mat4 modelview;
        glm::mat4 transformations;

        // events::EventManager eventManager;

        glm::vec3 translation;

        // std::vector<std::shared_ptr<Rectangle>> subRectangles;

        // std::vector<std::pair<std::weak_ptr<Rectangle>, std::shared_ptr<glm::mat4>>> inlinks;
        // std::vector<std::pair<std::weak_ptr<Rectangle>, std::shared_ptr<glm::mat4>>> outlinks;
        std::array<GLfloat, 8> vertices = {0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f};
        std::array<GLfloat, 8> texcoords = {0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f};
        static GLuint vBuffer, tBuffer, vArray;

        std::shared_ptr<Texture> texture;

        float angle = 0.0f;
        float w, h, x, y;

        Rectangle();
        Rectangle(float x, float y, float w, float h, std::shared_ptr<Texture> &&t = std::make_shared<Texture>());
        ~Rectangle() = default;

        Rectangle(const Rectangle &) = default;
        Rectangle(Rectangle &&) = default;

        // std::weak_ptr<Volume> allocVolume(unsigned int depth, unsigned int length, int unsigned width, const std::vector<uint8_t> &data);
        // void allocTexture(unsigned int w, unsigned int h);

        void draw() const
        {
            events::draw(*this);
        }

        void drawChildren() const;
        void drawLinks();

        void update();

        // bool contains(float x, float y) const;
        // bool containsTF(float x, float y) const;

        // bool contains(const Rectangle &rec) const;
        // bool containsTF(const Rectangle &rec) const;

        // static glm::vec4 globalMouse(Uint32);
        // glm::vec4 snapToEdge(const glm::vec4 &) const;

        // std::weak_ptr<Rectangle> addRectangle(float x, float y, float w, float h);

        // void showEvent(const SDL_Event &e);
        // void hideEvent(const SDL_Event &e);

        // void volumeStartEvent(const SDL_Event &e);
        // void dragStartEvent(const SDL_Event &e);
        // void moveStartEvent(const SDL_Event &e);
        // void dropEvent(const SDL_Event &e);
        // void stopEvent(const SDL_Event &e);
        // void linkEvent(const SDL_Event &e);

        // void updateArrow(const SDL_Event &e);

        // void scaleEvent(const SDL_Event &e);
        // void scrollEvent(const SDL_Event &e);

        // void doubleDown(const SDL_Event &e);
        // void doubleUp(const SDL_Event &e);

        // bool process([[maybe_unused]]const SDL_Event &e){return false;}

        gui::Rectangle &operator=(const gui::Rectangle &) = default;
        gui::Rectangle &operator=(gui::Rectangle &&) = default;
    };

} // namespace gui

#endif