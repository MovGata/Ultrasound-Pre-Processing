#ifndef GUI_KERNEL_HH
#define GUI_KERNEL_HH

#include <string>
#include <memory>
#include <variant>
#include <vector>

#include "Rectangle.hh"
#include "Button.hh"

#include "../OpenCL/Kernel.hh"
#include "../events/EventManager.hh"
#include "../OpenCL/Concepts.hh"

namespace gui
{
    template <typename K>
    class Kernel : public Rectangle, public std::enable_shared_from_this<Kernel<K>>
    {
    private:
        Kernel(std::shared_ptr<K> &&k) : Rectangle(0.0f, 0.0f, 40.0f, 40.0f), kernel(std::forward<std::shared_ptr<K>>(k))
        {
            texture->fill({0x5C, 0x5C, 0x5C, 0xFF});

            inNode = (Button<Rectangle>::build(kernel->in));
            inNode->onPress([str = kernel->in]()
                            { std::cout << str << std::endl; });

            outNode = (Button<Rectangle>::build(kernel->out));
            outNode->onPress([str = kernel->out]()
                             { std::cout << str << std::endl; });

            inNode->y = y + h / 2.0f;
            inNode->x = x + 2.0f;
            inNode->update();

            outNode->y = y + h / 2.0f;
            outNode->x = x + w - 2.0f - outNode->w;
            outNode->update();
        }

        std::shared_ptr<K> kernel;

        std::shared_ptr<Button<Rectangle>> inNode;
        std::shared_ptr<Button<Rectangle>> outNode;

    public:
        events::EventManager eventManager;

        static std::shared_ptr<Kernel<K>> build(std::shared_ptr<K> &&k)
        {
            auto sptr = std::shared_ptr<Kernel<K>>(new Kernel<K>(std::forward<std::shared_ptr<K>>(k)));
            sptr->eventManager.addCallback(
                SDL_MOUSEBUTTONDOWN,
                [wptr = sptr->weak_from_this()](const SDL_Event &e)
                {
                    auto ptr = wptr.lock();
                    if (events::containsMouse(std::as_const(*ptr->inNode), e))
                    {
                        ptr->inNode->eventManager.process(e);
                        return;
                    }
                    else if (events::containsMouse(std::as_const(*ptr->outNode), e))
                    {
                        ptr->outNode->eventManager.process(e);
                        return;
                    }
                });
            sptr->eventManager.addCallback(
                SDL_MOUSEMOTION,
                [wptr = sptr->weak_from_this()](const SDL_Event &e)
                {
                    auto ptr = wptr.lock();

                    auto dy = ptr->y;
                    auto dx = ptr->x;

                    ptr->x = static_cast<float>(e.motion.x) - ptr->w / 2.0f;
                    ptr->y = static_cast<float>(e.motion.y) - ptr->h / 2.0f;
                    ptr->update();

                    dy = ptr->y - dy;
                    dx = ptr->x - dx;

                    ptr->inNode->y += dy;
                    ptr->inNode->x += dx;
                    ptr->inNode->update();

                    ptr->outNode->y += dy;
                    ptr->outNode->x += dx;
                    ptr->outNode->update();
                });
            return sptr;
        }

        ~Kernel() = default;

        void draw()
        {
            Rectangle::draw();
            inNode->draw();
            outNode->draw();
        }

        template <typename WK, typename D>
        static std::shared_ptr<Button<Rectangle>> buildButton(const std::string &str, WK &wk, D &d, std::shared_ptr<K> &t)
        {
            auto button = Button<Rectangle>::build(str);
            
            button->onPress(
                [&sk = wk, &dropzone = d, k = t]() mutable
                {
                    std::shared_ptr<Kernel<K>> ptr = Kernel<K>::build(std::shared_ptr(k));
                    ptr->eventManager.addCallback(
                        SDL_MOUSEBUTTONUP,
                        [&sk, &dropzone](const SDL_Event &e)
                        {
                            if (events::containsMouse(*dropzone, e))
                            {
                                auto kptr = std::get<std::shared_ptr<Kernel<K>>>(sk);
                                kptr->y = std::max(dropzone->y, kptr->y);
                                kptr->y = std::min(dropzone->y + dropzone->h - kptr->h, kptr->y);
                                dropzone->kernels.emplace_back(kptr); // ADD DROPPED KERNEL FUNCTIONS HERE
                            }
                            std::get<std::shared_ptr<Kernel<K>>>(sk).template reset<Kernel<K>>(nullptr);
                        });
                    sk.template emplace<std::shared_ptr<Kernel<K>>>(std::move(ptr));
                });
            return button;
        }
    };

}
#endif