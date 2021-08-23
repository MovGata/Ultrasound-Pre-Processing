#ifndef GUI_RENDERER_HH
#define GUI_RENDERER_HH

#include <memory>

#include "Button.hh"
#include "Rectangle.hh"

#include "../Data/Volume.hh"

#include "../Events/Concepts.hh"
#include "../Events/EventManager.hh"

#include "../OpenCL/Concepts.hh"

#include "../OpenCL/Kernels/ToPolar.hh"
#include "../OpenCL/Kernels/ToCartesian.hh"
#include "../OpenCL/Kernels/Slice.hh"
#include "../OpenCL/Kernels/Invert.hh"
#include "../OpenCL/Kernels/Contrast.hh"
#include "../OpenCL/Kernels/Log2.hh"
#include "../OpenCL/Kernels/Shrink.hh"
#include "../OpenCL/Kernels/Fade.hh"
#include "../OpenCL/Kernels/Sqrt.hh"
#include "../OpenCL/Kernels/Clamp.hh"
#include "../OpenCL/Kernels/Threshold.hh"

#include "../Ultrasound/Mindray.hh"

namespace gui
{
    template <concepts::DrawableType Drawable, concepts::VolumeType TF>
    requires concepts::PositionableType<Drawable> && concepts::TranslatableType<TF>
    class Renderer;

    using namespace opencl;
    using ultrasound::Mindray;

    template <concepts::VolumeType R>
    using rk = std::shared_ptr<Renderer<Rectangle, R>>;

    using renType = std::variant<rk<ToPolar>, rk<ToCartesian>, rk<Slice>, rk<Threshold>, rk<Invert>, rk<Clamp>, rk<Contrast>, rk<Log2>, rk<Shrink>, rk<Fade>, rk<Sqrt>, rk<Mindray>>;

    template <concepts::DrawableType Drawable, concepts::VolumeType TF>
    requires concepts::PositionableType<Drawable> && concepts::TranslatableType<TF>
    class Renderer : public Drawable, public std::enable_shared_from_this<Renderer<Drawable, TF>>
    {
    private:
        std::shared_ptr<Button<Rectangle>> closeButton;

        Renderer(Drawable &&d, std::shared_ptr<TF> &&ptr) : Drawable(std::forward<Drawable>(d)), tf(std::forward<std::shared_ptr<TF>>(ptr)), eventManager(std::make_shared<events::EventManager>())
        {
            closeButton = Button<Rectangle>::build("x");
            closeButton->x = Drawable::x + Drawable::w - closeButton->w;
            closeButton->y = Drawable::y;
        }

    public:
        ~Renderer() = default;

        std::shared_ptr<TF> tf;

        std::shared_ptr<events::EventManager> eventManager;

        template <typename WR>
        static std::shared_ptr<Renderer<Drawable, TF>> build(WR &wr, Drawable &&d, std::shared_ptr<TF> &&ptr)
        {
            ptr->modified = true;
            auto rptr = std::shared_ptr<Renderer<Drawable, TF>>(new Renderer<Drawable, TF>(std::forward<Drawable>(d), std::forward<std::shared_ptr<TF>>(ptr)));

            rptr->closeButton->onPress([&wr, wptr = rptr->weak_from_this()]() mutable
                                       {
                                           auto sptr = wptr.lock();
                                           for (auto itr = wr.begin(); itr != wr.end(); itr = std::next(itr))
                                           {
                                               if (std::visit(
                                                       [&sptr](auto &&rndr)
                                                       {
                                                           if constexpr (std::same_as<std::decay_t<decltype(rndr)>, std::shared_ptr<Renderer<Drawable, TF>>>)
                                                           {
                                                               return sptr == rndr;
                                                           }

                                                           return false;
                                                       },
                                                       *itr))
                                               {
                                                   wr.erase(itr);
                                                   break;
                                               }
                                           }
                                       });

            rptr->eventManager->addCallback(
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
                    sptr->Drawable::update();

                    sptr->closeButton->x = sptr->x + sptr->w - sptr->closeButton->w;
                    sptr->closeButton->y = sptr->y;
                    sptr->closeButton->update();
                });
            rptr->eventManager->addCallback(
                SDL_MOUSEBUTTONDOWN,
                [wptr = rptr->weak_from_this()](const SDL_Event &e)
                {
                    auto sptr = wptr.lock();
                    if (e.button.button == SDL_BUTTON_LEFT)
                    {
                        if (events::containsMouse(std::as_const(*sptr->closeButton), e))
                        {
                            sptr->closeButton->eventManager->process(e);
                            return;
                        }

                        sptr->eventManager->addCallback(
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
                        sptr->eventManager->addCallback(
                            SDL_MOUSEMOTION, [wptr](const SDL_Event &ev)
                            {
                                auto ssptr = wptr.lock();
                                events::translate(*ssptr->tf, {-static_cast<float>(4 * ev.motion.xrel) / ssptr->w, -static_cast<float>(4 * ev.motion.yrel) / ssptr->h, 0.0f});
                            });
                        // sptr->eventManager->addCallback(SDL_MOUSEMOTION, events::translateEvent<TF>, *sptr->tf);
                    }
                });
            rptr->eventManager->addCallback(
                SDL_MOUSEBUTTONUP,
                [wptr = rptr->weak_from_this()]([[maybe_unused]] const SDL_Event &)
                {
                    auto sptr = wptr.lock();
                    sptr->eventManager->clearCallback(SDL_MOUSEMOTION);
                });
            rptr->eventManager->addCallback(
                SDL_MOUSEWHEEL,
                [wptr = rptr->weak_from_this()](const SDL_Event &e)
                {
                    auto sptr = wptr.lock();
                    events::translate(*sptr->tf, {0.0f, 0.0f, e.wheel.y});
                });
            return rptr;
        }

        void update(float xx, float yy)
        {
            Drawable::x = xx;
            Drawable::y = yy;
            Rectangle::update();
            closeButton->update(Drawable::x + Drawable::w - closeButton->w, Drawable::y);
        }

        void update(float xx, float yy, float ww, float hh)
        {
            Drawable::w = ww;
            Drawable::h = hh;
            update(xx, yy);
        }

        void draw()
        {
            Drawable::draw();
            closeButton->draw();
        }
    };

}

#endif