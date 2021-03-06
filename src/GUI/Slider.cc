#include <algorithm>
#include <iostream>
#include <cmath>

#include "Slider.hh"

namespace gui
{

    Slider::Slider(Rectangle &&rec) : Rectangle(std::forward<Rectangle>(rec)), bg(x + 1.0f, y + 1.0f, w - 1.0f, h - 1.0f), fg(x + 1.0f, y + 1.0f, 0.0f, h - 1.0f)
    {
        bg.texture->fill({0x60, 0x60, 0x60, 0xFF});
        fg.texture->fill({0xFF, 0x80, 0x80, 0xFF});
    }

    std::shared_ptr<Slider> Slider::build(float x, float y, float w, float h)
    {
        auto rptr = std::shared_ptr<Slider>(new Slider({x, y, w, h}));
        rptr->texture->fill({0x40, 0X40, 0X40, 0XFF});

        rptr->Rectangle::draw = std::bind(Slider::draw, rptr.get());
        rptr->Rectangle::resize = std::bind(Slider::update, rptr.get(), std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);

        rptr->eventManager->addCallback(
            SDL_MOUSEBUTTONDOWN, [wptr = rptr->weak_from_this()]([[maybe_unused]] const SDL_Event &)
            {
                auto ptr = wptr.lock();
                ptr->eventManager->addCallback(SDL_MOUSEMOTION,
                                               [wptr](const SDL_Event &e)
                                               {
                                                   auto pptr = wptr.lock();
                                                   pptr->modify((std::clamp(static_cast<float>(e.motion.x), pptr->fg.x, pptr->fg.x + pptr->bg.w) - pptr->fg.x) / (pptr->bg.w));
                                               });
            });

        rptr->eventManager->addCallback(
            SDL_MOUSEBUTTONUP, [wptr = rptr->weak_from_this()]([[maybe_unused]] const SDL_Event &)
            {
                auto ptr = wptr.lock();
                ptr->eventManager->clearCallback(SDL_MOUSEMOTION);
            });

        return rptr;
    }

    void Slider::modify(float p)
    {
        fg.w = std::lerp(0.0f, bg.w, p);
        fg.update();
        value = p;
    }

    void Slider::update(float xx, float yy, float ww, float hh)
    {
        Rectangle::update(xx, yy, ww, hh);
        bg.update(xx, yy, ww, hh);
        fg.update(xx, yy, 0.0f, hh);
        modify(value); // Scales progress to new width
    }

    void Slider::draw()
    {
        Rectangle::upload();
        bg.upload();
        fg.upload();
    }

} // namespace gui