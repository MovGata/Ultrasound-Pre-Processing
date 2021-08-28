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

#include "Texture.hh"

namespace gui
{
    class Rectangle
    {
    private:
        static std::once_flag onceFlag;
        static std::array<GLfloat, 8> vertices;
        static std::array<GLfloat, 8> texcoords;

    public:
        static GLuint vBuffer, tBuffer, vArray;

        Rectangle(std::shared_ptr<Texture> &&t = std::make_shared<Texture>());
        Rectangle(float x, float y, float w, float h, std::shared_ptr<Texture> &&t = std::make_shared<Texture>());
        
        bool hidden = false;
        glm::mat4 modelview;
        std::shared_ptr<events::EventManager> eventManager;
        std::shared_ptr<Texture> texture;

        float angle = 0.0f;
        float w, h, x, y;

        ~Rectangle() = default;

        Rectangle(const Rectangle &) = default;
        Rectangle(Rectangle &&) = default;

        void upload() const;
        std::function<void(void)> draw;
        std::function<void(float, float, float, float)> resize;

        void update(float x = 0.0f, float y = 0.0f, float w = 0.0f, float h = 0.0f);

        gui::Rectangle &operator=(const gui::Rectangle &) = default;
        gui::Rectangle &operator=(gui::Rectangle &&) = default;
    };

} // namespace gui

#endif