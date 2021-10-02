#ifndef GUI_BUTTON_HH
#define GUI_BUTTON_HH

#include "Rectangle.hh"

#include "../Events/Concepts.hh"
#include "../Events/GUI.hh"
#include "../Events/EventManager.hh"

namespace gui
{

    class Button : public Rectangle, public std::enable_shared_from_this<Button>
    {
    private:
        Button(Rectangle &&d);

    public:
        ~Button() = default;

        static std::shared_ptr<Button> build(const std::string &str);
        static std::shared_ptr<Button> build(Rectangle &&d);

        template <typename F, typename... Args>
        requires std::invocable<F, const SDL_Event &, Args &...>
        void onPress(F f, Args &&...args)
        {
            eventManager->addCallback(SDL_MOUSEBUTTONDOWN, f, std::forward<Args>(args)...);
        }

        template <typename F, typename... Args>
        requires std::invocable<F, Args &...>
        void onPress(F f, Args &&...args)
        {
            onPress([f, args...]([[maybe_unused]] const SDL_Event &e) mutable
                    { f(std::forward<Args>(args)...); });
        }

        template <typename F, typename... Args>
        requires std::invocable<F, const SDL_Event &, Args &...>
        void onRelease(F f, Args &&...args)
        {
            eventManager->addCallback(SDL_MOUSEBUTTONUP, f, std::forward<Args>(args)...);
        }

        template <typename F, typename... Args>
        requires std::invocable<F, Args &...>
        void onRelease(F f, Args &&...args)
        {
            onRelease([f, args...]([[maybe_unused]] const SDL_Event &e) mutable
                      { f(std::forward<Args>(args)...); });
        }
    };
}

#endif