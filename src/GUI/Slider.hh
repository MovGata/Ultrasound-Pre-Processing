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
        void update();
        void update(float x, float y);

        float value = 0.0f;
        std::shared_ptr<events::EventManager> eventManager;
        
        static std::shared_ptr<Slider> build(float x, float y, float w, float h);

    };

    
} // namespace gui


#endif