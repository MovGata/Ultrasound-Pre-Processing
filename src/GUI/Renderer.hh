#ifndef GUI_RENDERER_HH
#define GUI_RENDERER_HH

#include <memory>

#include "../Data/Volume.hh"
#include "../Events/Concepts.hh"
#include "../Events/EventManager.hh"
#include "Rectangle.hh"

namespace gui
{

    template <concepts::DrawableType Drawable, concepts::VolumeType TF>
    requires concepts::TranslatableType<TF>
    class Renderer : public Drawable, public std::enable_shared_from_this<Renderer<Drawable, TF>>
    {
    private:
        std::shared_ptr<TF> tf;

        Renderer(Drawable &&d, std::shared_ptr<TF> &&ptr) : Drawable(std::forward<Drawable>(d)), tf(std::forward<std::shared_ptr<TF>>(ptr))
        {
        }

    public:
        ~Renderer() = default;

        events::EventManager eventManager;

        static std::shared_ptr<Renderer<Drawable, TF>> build(Drawable &&d, std::shared_ptr<TF> &&ptr)
        {
            auto rptr = std::shared_ptr<Renderer<Drawable, TF>>(new Renderer<Drawable, TF>(std::forward<Drawable>(d), std::forward<std::shared_ptr<TF>>(ptr)));
            rptr->eventManager.addCallback(
                events::GUI_REDRAW, [wptr = rptr->weak_from_this()](const SDL_Event &e)
                {
                    auto sptr = wptr.lock();

                    std::pair<float, float> *newSize = static_cast<std::pair<float, float> *>(e.user.data2);

                    sptr->x = (newSize->first - std::max(newSize->first, newSize->second)) / 2.0f;
                    sptr->y = (newSize->second - std::max(newSize->first, newSize->second)) / 2.0f;
                    sptr->w = newSize->first;
                    sptr->h = newSize->second;

                    sptr->h = sptr->w = std::max(sptr->w, sptr->h);

                    // Keep the render shaped properly by cutting off the smaller edge.
                    sptr->update();
                });
            rptr->eventManager.addCallback(
                SDL_MOUSEBUTTONDOWN,
                [wptr = rptr->weak_from_this()](const SDL_Event &e)
                {
                    auto sptr = wptr.lock();
                    if (e.button.button == SDL_BUTTON_LEFT)
                    {
                        sptr->eventManager.addCallback(
                            SDL_MOUSEMOTION, [wptr](const SDL_Event &ev)
                            {
                                auto ssptr = wptr.lock();
                                ssptr->tf->lastview = glm::rotate(ssptr->tf->lastview, glm::radians(static_cast<float>(ev.motion.yrel)), {1.0f, 0.0f, 0.0f});
                                ssptr->tf->lastview = glm::rotate(ssptr->tf->lastview, -glm::radians(static_cast<float>(ev.motion.xrel)), {0.0f, 1.0f, 0.0f});
                                ssptr->tf->modified = true;
                            });
                    }
                    else if (e.button.button == SDL_BUTTON_RIGHT)
                    {
                        sptr->eventManager.addCallback(
                            SDL_MOUSEMOTION, [wptr](const SDL_Event &ev)
                            {
                                auto ssptr = wptr.lock();
                                events::translate(*ssptr->tf, {-static_cast<float>(4*ev.motion.xrel)/ssptr->w, -static_cast<float>(4*ev.motion.yrel)/ssptr->h, 0.0f});
                            });
                        // sptr->eventManager.addCallback(SDL_MOUSEMOTION, events::translateEvent<TF>, *sptr->tf);
                    }
                });
            rptr->eventManager.addCallback(
                SDL_MOUSEBUTTONUP,
                [wptr = rptr->weak_from_this()]([[maybe_unused]] const SDL_Event &)
                {
                    auto sptr = wptr.lock();
                    sptr->eventManager.clearCallback(SDL_MOUSEMOTION);
                });
            rptr->eventManager.addCallback(
                SDL_MOUSEWHEEL,
                [wptr = rptr->weak_from_this()](const SDL_Event &e)
                {
                    auto sptr = wptr.lock();
                    events::translate(*sptr->tf, {0.0f, 0.0f, e.wheel.y});
                });
            return rptr;
        }
    };

}

#endif