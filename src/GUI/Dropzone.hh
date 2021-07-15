#ifndef GUI_DROPZONE_HH
#define GUI_DROPZONE_HH

#include <memory>

#include "Button.hh"
#include "Rectangle.hh"
#include "Texture.hh"

#include "../Events/Concepts.hh"
#include "../Events/EventManager.hh"

namespace gui
{

    template <concepts::HidableType Drawable>
    class Dropzone : public Drawable, public std::enable_shared_from_this<Dropzone<Drawable>>
    {
    private:
        Dropzone(Drawable &&d) : Drawable(std::forward<Drawable>(d))
        {
            
        }

    public:
        ~Dropzone() = default;

        events::EventManager eventManager;

        void draw() const
        {
            if (!Drawable::hidden)
                Drawable::draw();
        }

        static std::shared_ptr<Dropzone<Drawable>> build(Drawable &&d)
        {
            auto rptr = std::shared_ptr<Dropzone<Drawable>>(new Dropzone<Drawable>(std::forward<Drawable>(d)));
            rptr->eventManager.addCallback(
                events::GUI_REDRAW, [wptr = rptr->weak_from_this()](const SDL_Event &e)
                {
                    auto ptr = wptr.lock();

                    std::pair<float, float> *oldSize = static_cast<std::pair<float, float> *>(e.user.data1);
                    std::pair<float, float> *newSize = static_cast<std::pair<float, float> *>(e.user.data2);

                    auto xd = ptr->x * newSize->first / oldSize->first;
                    auto yd = newSize->second / oldSize->second * (ptr->y + ptr->h) - ptr->h;
                    ptr->w = ptr->w * newSize->first / oldSize->first;
                    ptr->x = xd;
                    ptr->y = yd;
                    ptr->update();
                });
            return rptr;
        }
    };

}

#endif