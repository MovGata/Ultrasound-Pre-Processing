#ifndef GUI_RENDERER_HH
#define GUI_RENDERER_HH

#include <memory>

#include "../Events/Concepts.hh"
#include "../Events/EventManager.hh"
#include "Rectangle.hh"

namespace gui
{

    template <concepts::DrawableType Drawable>
    class Renderer : public Drawable, public std::enable_shared_from_this<Renderer<Drawable>>
    {
    private:
        Renderer(Drawable &&d) : Drawable(std::forward<Drawable>(d)){}

    public:
        ~Renderer()=default;

        events::EventManager eventManager;

        static std::shared_ptr<Renderer<Drawable>> build(Drawable &&d)
        {
            auto rptr = std::shared_ptr<Renderer<Drawable>>(new Renderer<Drawable>(std::forward<Drawable>(d)));
            rptr->eventManager.addCallback(
                events::GUI_REDRAW, [wptr = rptr->weak_from_this()](const SDL_Event &e)
                {
                    auto ptr = wptr.lock();

                    std::pair<float, float> *newSize = static_cast<std::pair<float, float> *>(e.user.data2);

                    ptr->x = (newSize->first - std::max(newSize->first, newSize->second))/2.0f;
                    ptr->y = (newSize->second - std::max(newSize->first, newSize->second))/2.0f;
                    ptr->w = newSize->first;
                    ptr->h = newSize->second;

                    ptr->h = ptr->w = std::max(ptr->w, ptr->h);

                    // Keep the render shaped properly by cutting off the smaller edge.
                    ptr->update();
                });
            return rptr;
        }

    };

}

#endif