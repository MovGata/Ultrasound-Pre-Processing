#ifndef GUI_KERNEL_HH
#define GUI_KERNEL_HH

#include <string>
#include <memory>
#include <variant>
#include <vector>

#include <glm/ext.hpp>

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
        Kernel(std::shared_ptr<K> &&k) : Rectangle(0.0f, 0.0f, 40.0f, 40.0f), kernel(std::forward<std::shared_ptr<K>>(k)), outLine({0.0f, 0.0f, 1.0f, 1.0f})
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

            outLine.hidden = true;
            outLine.texture->fill({0xD3, 0xD3, 0xD3, 0xFF});
        }

        std::shared_ptr<K> kernel;

    public:
        std::shared_ptr<Button<Rectangle>> inNode;
        std::shared_ptr<Button<Rectangle>> outNode;
        events::EventManager eventManager;
        gui::Rectangle outLine;

        bool link = false;

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
                        ptr->link = true;
                        ptr->inNode->eventManager.process(e);
                        return;
                    }
                    else if (events::containsMouse(std::as_const(*ptr->outNode), e))
                    {
                        ptr->link = true;
                        ptr->outLine.hidden = false;
                        ptr->outNode->eventManager.process(e);
                        return;
                    }
                    else
                    {
                        ptr->link = false;
                    }
                });
            sptr->eventManager.addCallback(
                SDL_MOUSEMOTION,
                [wptr = sptr->weak_from_this()](const SDL_Event &e)
                {
                    auto ptr = wptr.lock();

                    if (ptr->link)
                    {
                        ptr->outLine.y = ptr->outNode->y + ptr->outNode->w / 2.0f - ptr->outLine.h / 2.0f;
                        ptr->outLine.x = ptr->x + ptr->w;
                        ptr->outLine.angle = std::atan2(static_cast<float>(e.motion.y) - ptr->outLine.y, static_cast<float>(e.motion.x) - ptr->outLine.x);
                        ptr->outLine.h = 3.0f;
                        ptr->outLine.w = std::sqrt((static_cast<float>(e.motion.y) - ptr->outLine.y) * (static_cast<float>(e.motion.y) - ptr->outLine.y) + (static_cast<float>(e.motion.x) - ptr->outLine.x) * (static_cast<float>(e.motion.x) - ptr->outLine.x));
                        ptr->outLine.update();
                    }
                    else
                    {
                        auto dy = ptr->y;
                        auto dx = ptr->x;

                        ptr->x = static_cast<float>(e.motion.x) - ptr->w / 2.0f;
                        ptr->y = static_cast<float>(e.motion.y) - ptr->h / 2.0f;
                        ptr->Rectangle::update();

                        dy = ptr->y - dy;
                        dx = ptr->x - dx;

                        ptr->inNode->y += dy;
                        ptr->inNode->x += dx;
                        ptr->inNode->update();

                        ptr->outNode->y += dy;
                        ptr->outNode->x += dx;
                        ptr->outNode->update();
                    }
                });
            return sptr;
        }

        ~Kernel() = default;

        void draw()
        {
            Rectangle::draw();
            inNode->draw();
            outNode->draw();

            if (!outLine.hidden)
            {
                outLine.draw();
            }
        }

        void handleMouse(const SDL_Event &e)
        {
            if (e.button.type == SDL_MOUSEBUTTONDOWN)
            {
                if (events::containsMouse(*outNode, e))
                {
                    outLine.hidden = false;
                }
            }
            else if (e.button.type == SDL_MOUSEBUTTONUP)
            {
                outLine.hidden = true;
            }
        }

        void update(float dy)
        {
            y += dy;
            Rectangle::update();
            inNode->y += dy;
            outNode->y += dy;
            outLine.y += dy;

            inNode->update();
            outNode->update();
            outLine.update();
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

                                dropzone->kernels.emplace_back(kptr);

                                kptr->eventManager.addCallback(
                                    SDL_MOUSEBUTTONDOWN,
                                    [wptr = kptr->weak_from_this(), &dropzone, &sk]([[maybe_unused]] const SDL_Event &ev)
                                    {
                                        auto cptr = wptr.lock();
                                        sk.template emplace<std::shared_ptr<Kernel<K>>>(cptr);
                                        cptr->eventManager.clearCallback(SDL_MOUSEBUTTONUP);
                                        cptr->eventManager.addCallback(
                                            SDL_MOUSEBUTTONUP,
                                            [&dropzone, kptr = cptr->weak_from_this()](const SDL_Event &evv)
                                            {
                                                auto skptr = kptr.lock();
                                                if (skptr->link)
                                                {
                                                    for (auto &kernel : dropzone->kernels)
                                                    {
                                                        if (std::visit(
                                                                [&evv, wptr2 = skptr->weak_from_this()](auto &&krnl)
                                                                {
                                                                    auto sptr2 = wptr2.lock();
                                                                    if constexpr (std::same_as<std::decay_t<decltype(krnl)>, std::shared_ptr<Kernel<K>>>)
                                                                    {
                                                                        if ((krnl.get() != sptr2.get()) && events::containsMouse(*krnl->inNode, evv))
                                                                        {
                                                                            
                                                                            sptr2->outLine.hidden = false;
                                                                            return true;
                                                                        }
                                                                        else
                                                                        {
                                                                            sptr2->link = false;
                                                                            sptr2->outLine.hidden = true;
                                                                        }
                                                                    }
                                                                    else
                                                                    {
                                                                        if (events::containsMouse(*krnl->inNode, evv))
                                                                        {
                                                                            std::cout << "LINK MADE" << std::endl;
                                                                            sptr2->outLine.hidden = false;
                                                                            return true;
                                                                        }
                                                                        else
                                                                        {
                                                                            sptr2->link = false;
                                                                            sptr2->outLine.hidden = true;
                                                                        }
                                                                    }
                                                                    return false;
                                                                },
                                                                kernel))
                                                        {
                                                            break;
                                                        }
                                                    }
                                                }
                                            });
                                        cptr->eventManager.addCallback(
                                            SDL_MOUSEBUTTONUP,
                                            [&sk]([[maybe_unused]] const SDL_Event &evv)
                                            {
                                                std::get<std::shared_ptr<Kernel<K>>>(sk).template reset<Kernel<K>>(nullptr);
                                            });
                                    });
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