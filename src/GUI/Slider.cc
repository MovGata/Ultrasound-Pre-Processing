#include <algorithm>

#include "Slider.hh"

namespace gui
{

    Slider::Slider(Rectangle &&rec) : Rectangle(std::forward<Rectangle>(rec)), bg(x + 1.0f, y + 1.0f, w - 2.0f, h - 2.0f), fg(x + 1.0f, y + 1.0f, 0.0f, 0.0f)
    {
        bg.texture->fill({0x60, 0x60, 0x60, 0xFF});
        fg.texture->fill({0x80, 0x80, 0x80, 0xFF});
    }

    std::shared_ptr<Slider> Slider::build(float x, float y, float w, float h)
    {
        auto rptr = std::shared_ptr<Slider>(new Slider({x, y, w, h}));
        rptr->texture->fill({0x40, 0X40, 0X40, 0XFF});

        rptr->eventManager.addCallback(
            SDL_MOUSEBUTTONDOWN, [wptr = rptr->weak_from_this()]([[maybe_unused]]const SDL_Event &)
            {
                auto ptr = wptr.lock();
                ptr->eventManager.addCallback(SDL_MOUSEMOTION, 
                                                  [wptr](const SDL_Event &e)
                                              {
                                                  auto pptr = wptr.lock();
                                                  
                                                  pptr->fg.w = std::clamp(static_cast<float>(e.motion.x), pptr->fg.x, pptr->bg.w - 1.0f);
                                                  pptr->fg.update();

                                                  pptr->value = std::clamp((pptr->fg.w - pptr->fg.x) / (pptr->bg.w - 2.0f), 0.0f, 1.0f);
                                              });
            });

        rptr->eventManager.addCallback(
            SDL_MOUSEBUTTONUP, [wptr = rptr->weak_from_this()]([[maybe_unused]]const SDL_Event &)
            {
                auto ptr = wptr.lock();
                ptr->eventManager.clearCallback(SDL_MOUSEMOTION);
            });

        return rptr;
    }

    void Slider::draw()
    {
        Rectangle::draw();
        bg.draw();
        fg.draw();
    }

} // namespace gui