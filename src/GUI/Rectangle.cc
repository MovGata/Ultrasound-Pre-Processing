#include "Rectangle.hh"

#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glext.h>

#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "../Events/GUI.hh"

namespace gui
{
    std::once_flag Rectangle::onceFlag;

    std::array<GLfloat, 8> Rectangle::vertices = {0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f};
    std::array<GLfloat, 8> Rectangle::texcoords = {0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f};

    GLuint Rectangle::vBuffer = 0;
    GLuint Rectangle::tBuffer = 0;
    GLuint Rectangle::vArray = 0;

    Rectangle::Rectangle(std::shared_ptr<Texture> &&t) : Rectangle(0.0f, 0.0f, 1.0f, 1.0f, std::move(t)) {}

    Rectangle::Rectangle(float xp, float yp, float wp, float hp, std::shared_ptr<Texture> &&t)
        : modelview(1.0f), eventManager(std::make_shared<events::EventManager>()), texture(std::forward<std::shared_ptr<Texture>>(t)),
          w(wp), h(hp), x(xp), y(yp)
    {
        std::call_once(
            onceFlag,
            [this]
            {
                glGenBuffers(1, &vBuffer);
                glBindBuffer(GL_ARRAY_BUFFER, vBuffer);
                glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);

                glGenBuffers(1, &tBuffer);
                glBindBuffer(GL_ARRAY_BUFFER, tBuffer);
                glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(GLfloat), texcoords.data(), GL_STATIC_DRAW);

                glGenVertexArrays(1, &vArray);
                glBindVertexArray(vArray);

                glEnableVertexAttribArray(0);
                glBindBuffer(GL_ARRAY_BUFFER, vBuffer);
                glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

                glEnableVertexAttribArray(1);
                glBindBuffer(GL_ARRAY_BUFFER, tBuffer);
                glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

                glBindBuffer(GL_ARRAY_BUFFER, 0);
                glBindVertexArray(0);
            });
        update();
        draw = std::bind(Rectangle::upload, this);
    }

    void Rectangle::upload() const
    {
        if (!hidden)
            events::draw(*this);
    }

    void Rectangle::update(float xx, float yy, float ww, float hh)
    {
        w += ww;
        h += hh;
        x += xx;
        y += yy;

        glm::mat4 id(1.0f);
        modelview = id;
        modelview = glm::scale(id, {w, h, 1.0}) * modelview;
        modelview = glm::rotate(id, angle, {0.0f, 0.0f, 1.0f}) * modelview;
        modelview = glm::translate(id, {x, y, 0.0}) * modelview;
    }

} // namespace gui