#ifndef GUI_RECTANGLE_HH
#define GUI_RECTANGLE_HH

namespace gui
{

    class Rectangle
    {
    private:
        unsigned int x, y, w, h;
    public:
        Rectangle(unsigned int x, unsigned int y, unsigned int w, unsigned int h);
        ~Rectangle();

        void render() const;
    };

} // namespace gui

#endif