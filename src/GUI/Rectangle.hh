#ifndef GUI_RECTANGLE_HH
#define GUI_RECTANGLE_HH

#include <GL/glew.h>
#include <GL/gl.h>

#include <variant>

namespace gui
{

    class Rectangle
    {
    private:
    public:
        GLuint texture = 0;
        GLuint pixelBuffer = 0;
        GLsizei ww = 0, hh = 0;
        float x, y, w, h;
        Rectangle(float x, float y, float w, float h);
        ~Rectangle();

        void allocTexture(unsigned int w, unsigned int h);
        void render() const;
    };

} // namespace gui

#endif