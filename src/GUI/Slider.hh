#ifndef GUI_SLIDER_HH
#define GUI_SLIDER_HH

#include <memory>

#include "Rectangle.hh"
#include "../Events/EventManager.hh"

namespace gui
{
    
    class Slider : public Rectangle, public std::enable_shared_from_this<Slider>
    {
    private:
        Slider(Rectangle &&r);
        Rectangle bg;
        Rectangle fg;

        
    public:
        ~Slider() = default;

        void draw();
        void update(float xx = 0.0f, float yy = 0.0f, float ww = 0.0f, float hh = 0.0f);

        void modify(float p);

        float value = 0.0f;
        
        static std::shared_ptr<Slider> build(float x, float y, float w, float h);

    };

    
} // namespace gui


#endif