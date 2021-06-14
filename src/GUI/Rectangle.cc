#include "Rectangle.hh"

#include <GL/gl.h>

namespace gui
{

    Rectangle::Rectangle(unsigned int xp, unsigned int yp, unsigned int wp, unsigned int hp) : x(xp), y(yp), w(wp), h(hp)
    {
    }

    Rectangle::~Rectangle()
    {
    }

    void Rectangle::render() const
    {
        glBegin(GL_QUADS);
        glColor3f(0.4f, 0.4f, 0.4f);
        glVertex2i(x, y);
        glVertex2i(x + w, y);
        glVertex2i(x + w, y + h);
        glVertex2i(x, y + h);
        glEnd();
    }

} // namespace gui