#ifndef GUI_BUTTON_HH
#define GUI_BUTTON_HH

#include "../Events/Concepts.hh"

#include "../Events/EventManager.hh"

namespace gui
{

    template <concepts::DrawableType Drawable>
    class Button : public Drawable, public std::enable_shared_from_this<Button<Drawable>>
    {
    private:
        Button(Drawable &&d) : Drawable(std::forward<Drawable>(d))
        {
        }

    public:
        ~Button() = default;

        static std::shared_ptr<Button<Drawable>> build(Drawable &&d)
        {
            auto rptr = std::shared_ptr<Button<Drawable>>(new Button<Drawable>(std::forward<Drawable>(d)));
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

                    // Keep the render shaped properly by cutting off the smaller edge.
                    ptr->update();
                });
            return rptr;
        }

        events::EventManager eventManager;

        template <typename F, typename... Args>
        requires std::invocable<F, const SDL_Event &, const Args &...>
        void onPress(const F &f, const Args &...args)
        {
            eventManager.addCallback(SDL_MOUSEBUTTONDOWN, f, args...);
        }

        template <typename F, typename... Args>
        requires std::invocable<F, const Args &...>
        void onPress(const F &f, const Args &...args)
        {
            onPress([f, args...]([[maybe_unused]] const SDL_Event &e)
                    { f(args...); });
        }

        template <typename F, typename... Args>
        requires std::invocable<F, const SDL_Event &, const Args &...>
        void onRelease(const F &f, const Args &...args)
        {
            eventManager.addCallback(SDL_MOUSEBUTTONUP, f, args...);
        }

        template <typename F, typename... Args>
        requires std::invocable<F, const Args &...>
        void onRelease(const F &f, const Args &...args)
        {
            onRelease([f, args...]([[maybe_unused]] const SDL_Event &e)
                      { f(args...); });
        }
    };
}

#endif