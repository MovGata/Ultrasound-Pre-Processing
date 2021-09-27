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
        Button(Rectangle &&d) : Rectangle(std::forward<Rectangle>(d))
        {
        }

    public:
        ~Button() = default;

        static std::shared_ptr<Button> build(const std::string &str)
        {
            int w, h;
            TTF_SizeText(Texture::lastFont, str.c_str(), &w, &h);
            auto t = std::make_shared<gui::Texture>(w + 2, h + 2);
            t->addText(Texture::lastFont, str.c_str());
            return build({0.0f, 0.0f, static_cast<float>(w) + 2, static_cast<float>(h) + 2, std::move(t)});
        }

        static std::shared_ptr<Button> build(Rectangle &&d)
        {
            auto rptr = std::shared_ptr<Button>(new Button(std::forward<Rectangle>(d)));
            rptr->Rectangle::draw = std::bind(Rectangle::upload, rptr.get());
            rptr->Rectangle::resize = std::bind(Rectangle::update, rptr.get(), std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);

            rptr->eventManager->addCallback(
                events::GUI_REDRAW, [wptr = rptr->weak_from_this()](const SDL_Event &e)
                {
                    auto ptr = wptr.lock();

                    std::pair<float, float> *newSize = static_cast<std::pair<float, float> *>(e.user.data2);

                    ptr->resize(
                        newSize->first - ptr->w - ptr->x,
                        newSize->second - ptr->h - ptr->y,
                        0.0f,// ptr->w * newSize->first / oldSize->first - ptr->w,
                        0.0f);

                    // Keep the render shaped properly by cutting off the smaller edge.
                    // ptr->resize();
                });
            return rptr;
        }

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