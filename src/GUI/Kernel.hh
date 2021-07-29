#ifndef GUI_KERNEL_HH
#define GUI_KERNEL_HH

#include <string>
#include <memory>
#include <variant>
#include <vector>

#include <glm/ext.hpp>

#include "Rectangle.hh"
#include "Button.hh"
#include "Texture.hh"
#include "Renderer.hh"

#include "../OpenCL/Kernel.hh"
#include "../events/EventManager.hh"
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


namespace
{
    using namespace opencl;
    using namespace ultrasound;
}

namespace gui
{
    template <typename K>
    class Kernel;

    template <typename KK>
    using sk = std::shared_ptr<Kernel<KK>>;
    
    using varType = std::variant<sk<ToPolar>, sk<ToCartesian>, sk<Slice>, sk<Threshold>, sk<Invert>, sk<Clamp>, sk<Contrast>, sk<Log2>, sk<Shrink>, sk<Fade>, sk<Sqrt>, sk<Mindray>>;

    template <typename K>
    class Kernel : public Rectangle, public std::enable_shared_from_this<Kernel<K>>
    {
    private:


        Kernel(std::shared_ptr<K> &&k, std::shared_ptr<Texture> &&tptr) : Rectangle(0.0f, 0.0f, 40.0f, 40.0f), kernel(std::forward<std::shared_ptr<K>>(k)), outLine({0.0f, 0.0f, 1.0f, 1.0f}), title(0.0f, 0.0f, static_cast<float>(tptr->textureW), static_cast<float>(tptr->textureH), std::forward<std::shared_ptr<Texture>>(tptr))
        {
            texture->fill({0x5C, 0x5C, 0x5C, 0xFF});

            inNode = (Button<Rectangle>::build(kernel->in));

            outNode = (Button<Rectangle>::build(kernel->out));

            w = std::max(w, title.w);
            h = title.h * 3.0f;

            title.x = x + w / 2.0f - title.w / 2.0f;
            title.y = y;

            inNode->y = y + h / 2.0f;
            inNode->x = x + 2.0f;
            inNode->update();

            outNode->y = y + h / 2.0f;
            outNode->x = x + w - 2.0f - outNode->w;
            outNode->update();

            outLine.hidden = true;
            outLine.texture->fill({0xD3, 0xD3, 0xD3, 0xFF});
        }

    public:
        std::shared_ptr<K> kernel;
        std::shared_ptr<Button<Rectangle>> inNode;
        std::shared_ptr<Button<Rectangle>> outNode;

        gui::Rectangle outLine;
        gui::Rectangle title;

        std::shared_ptr<varType> inLink;
        std::shared_ptr<varType> outLink;

        events::EventManager eventManager;

        bool link = false;

        static std::shared_ptr<Kernel<K>> build(std::shared_ptr<K> &&k, std::shared_ptr<Texture> &&tptr)
        {
            auto sptr = std::shared_ptr<Kernel<K>>(new Kernel<K>(std::forward<std::shared_ptr<K>>(k), std::forward<std::shared_ptr<Texture>>(tptr)));
            sptr->eventManager.addCallback(
                SDL_MOUSEBUTTONDOWN,
                [wptr = sptr->weak_from_this()](const SDL_Event &e)
                {
                    auto ptr = wptr.lock();
                    if (ptr->kernel->in == "IN" && events::containsMouse(std::as_const(*ptr->inNode), e))
                    {
                        if (e.button.clicks == 2)
                        {
                            ptr->execute();
                        }
                    }
                    else if (events::containsMouse(std::as_const(*ptr->outNode), e))
                    {
                        if (ptr->outLink)
                        {
                            std::visit([](auto &&krnl)
                                       { krnl->inLink.reset(); },
                                       *ptr->outLink);
                        }

                        ptr->outLine.y = ptr->outNode->y + ptr->outNode->h / 2.0f - ptr->outLine.h / 2.0f;
                        ptr->outLine.x = ptr->x + ptr->w;
                        ptr->outLine.angle = std::atan2(static_cast<float>(e.motion.y) - ptr->outLine.y, static_cast<float>(e.motion.x) - ptr->outLine.x);
                        ptr->outLine.h = 3.0f;
                        ptr->outLine.w = std::sqrt((static_cast<float>(e.motion.y) - ptr->outLine.y) * (static_cast<float>(e.motion.y) - ptr->outLine.y) + (static_cast<float>(e.motion.x) - ptr->outLine.x) * (static_cast<float>(e.motion.x) - ptr->outLine.x));
                        ptr->outLine.update();

                        ptr->outLine.hidden = false;
                        ptr->link = true;
                        ptr->outNode->eventManager.process(e);
                        return;
                    }
                    else
                    {
                        ptr->link = false;
                        if (e.button.clicks == 2)
                        {
                        }
                    }
                });

            sptr->eventManager.addCallback(
                SDL_MOUSEMOTION,
                [wptr = sptr->weak_from_this()](const SDL_Event &e)
                {
                    auto ptr = wptr.lock();

                    if (ptr->link)
                    {
                        ptr->outLine.y = ptr->outNode->y + ptr->outNode->h / 2.0f - ptr->outLine.h / 2.0f;
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

                        ptr->title.y += dy;
                        ptr->title.x += dx;
                        ptr->title.update();

                        if (ptr->outLink)
                        {
                            float oy = std::visit([](auto &&l)
                                                  { return l->inNode->y + l->inNode->h / 2.0f; },
                                                  *ptr->outLink);
                            float ox = std::visit([](auto &&l)
                                                  { return l->inNode->x; },
                                                  *ptr->outLink);

                            ptr->outLine.y = ptr->outNode->y + ptr->outNode->h / 2.0f - ptr->outLine.h / 2.0f;
                            ptr->outLine.x = ptr->x + ptr->w;
                            ptr->outLine.angle = std::atan2(oy - ptr->outLine.y, ox - ptr->outLine.x);
                            ptr->outLine.h = 3.0f;
                            ptr->outLine.w = std::sqrt((oy - ptr->outLine.y) * (oy - ptr->outLine.y) + (ox - ptr->outLine.x) * (ox - ptr->outLine.x));
                            ptr->outLine.update();
                        }

                        if (ptr->inLink)
                        {
                            std::visit([oy = ptr->inNode->y + ptr->inNode->h / 2.0f, ox = ptr->inNode->x + ptr->inNode->w](auto &&l)
                                       {
                                           l->outLine.y = l->outNode->y + l->outNode->h / 2.0f - l->outLine.h / 2.0f;
                                           l->outLine.x = l->x + l->w;
                                           l->outLine.angle = std::atan2(oy - l->outLine.y, ox - l->outLine.x);
                                           l->outLine.h = 3.0f;
                                           l->outLine.w = std::sqrt((oy - l->outLine.y) * (oy - l->outLine.y) + (ox - l->outLine.x) * (ox - l->outLine.x));
                                           l->outLine.update();
                                       },
                                       *ptr->inLink);
                        }
                    }
                });
            return sptr;
        }

        ~Kernel() = default;

        void draw()
        {
            Rectangle::draw();
            title.draw();
            inNode->draw();
            outNode->draw();

            if (!outLine.hidden)
            {
                outLine.draw();
            }
        }

        void execute()
        {
            if (inLink)
            {
                std::visit(
                    [this](auto &&k)
                    { kernel->template input<std::decay_t<decltype(*k->kernel)>>(*k->kernel); },
                    *inLink);
            }

            kernel->execute();

            if (outLink)
            {
                std::visit(
                    [](auto &&k)
                    { k->execute(); },
                    *outLink);
            }
        }

        void update(float dy)
        {
            y += dy;
            Rectangle::update();
            inNode->y += dy;
            outNode->y += dy;
            outLine.y += dy;
            title.y += dy;

            inNode->update();
            outNode->update();
            outLine.update();
            title.update();
        }

        template <typename KT>
        static bool endLink(const SDL_Event &e, std::weak_ptr<Kernel<K>> wptr, std::shared_ptr<KT> &k)
        {
            auto ptr = wptr.lock();
            if constexpr (std::same_as<std::decay_t<decltype(k)>, std::shared_ptr<Kernel<K>>>)
            {
                if ((k.get() != ptr.get()) && ptr->kernel->out == k->kernel->in && events::containsMouse(*k->inNode, e))
                {
                    if (k->inLink)
                    {
                        std::visit([](auto &&krnl)
                                   {
                                       krnl->outLink.reset();
                                       krnl->outLine.hidden = true;
                                   },
                                   *k->inLink);
                    }

                    float oy = k->inNode->y + k->inNode->h / 2.0f;
                    float ox = k->inNode->x;
                    ptr->outLine.y = ptr->outNode->y + ptr->outNode->h / 2.0f - ptr->outLine.h / 2.0f;
                    ptr->outLine.x = ptr->x + ptr->w;
                    ptr->outLine.angle = std::atan2(oy - ptr->outLine.y, ox - ptr->outLine.x);
                    ptr->outLine.h = 3.0f;
                    ptr->outLine.w = std::sqrt((oy - ptr->outLine.y) * (oy - ptr->outLine.y) + (ox - ptr->outLine.x) * (ox - ptr->outLine.x));
                    ptr->outLine.update();

                    ptr->outLink = std::make_shared<varType>(k);
                    k->inLink = std::make_shared<varType>(ptr);
                    ptr->outLine.hidden = false;
                    return true;
                }
                else
                {
                    ptr->link = false;
                    ptr->outLine.hidden = true;
                }
            }
            else
            {
                if (ptr->kernel->out == k->kernel->in && events::containsMouse(*k->inNode, e))
                {
                    if (k->inLink)
                    {
                        std::visit([](auto &&krnl)
                                   {
                                       krnl->outLink.reset();
                                       krnl->outLine.hidden = true;
                                   },
                                   *k->inLink);
                    }

                    float oy = k->inNode->y + k->inNode->h / 2.0f;
                    float ox = k->inNode->x;
                    ptr->outLine.y = ptr->outNode->y + ptr->outNode->h / 2.0f - ptr->outLine.h / 2.0f;
                    ptr->outLine.x = ptr->x + ptr->w;
                    ptr->outLine.angle = std::atan2(oy - ptr->outLine.y, ox - ptr->outLine.x);
                    ptr->outLine.h = 3.0f;
                    ptr->outLine.w = std::sqrt((oy - ptr->outLine.y) * (oy - ptr->outLine.y) + (ox - ptr->outLine.x) * (ox - ptr->outLine.x));
                    ptr->outLine.update();
                    ptr->outLink = std::make_shared<varType>(k);
                    k->inLink = std::make_shared<varType>(ptr);
                    ptr->outLine.hidden = false;
                    return true;
                }
                else
                {
                    ptr->link = false;
                    ptr->outLine.hidden = true;
                }
            }
            return false;
        }

        template <typename WK, typename WR, typename D>
        static std::shared_ptr<Button<Rectangle>> buildButton(const std::string &str, WK &wk, WR &wr, D &d, std::shared_ptr<K> &t)
        {
            auto button = Button<Rectangle>::build(str);

            button->onPress(
                [&sk = wk, &wr, &dropzone = d, k = t, wptr = std::weak_ptr<Texture>(button->texture)]() mutable
                {
                    std::shared_ptr<Kernel<K>> ptr = Kernel<K>::build(std::shared_ptr(k), wptr.lock());
                    ptr->eventManager.addCallback(
                        SDL_MOUSEBUTTONUP,
                        [&sk, &wr, &dropzone](const SDL_Event &e)
                        {
                            if (events::containsMouse(*dropzone, e))
                            {
                                auto kptr = std::get<std::shared_ptr<Kernel<K>>>(sk);
                                kptr->y = std::max(dropzone->y, kptr->y);
                                kptr->y = std::min(dropzone->y + dropzone->h - kptr->h, kptr->y);

                                dropzone->kernels.emplace_back(std::make_shared<varType>(kptr));

                                kptr->eventManager.addCallback(
                                    SDL_MOUSEBUTTONDOWN,
                                    [wptr = kptr->weak_from_this(), &dropzone, &sk, &wr]([[maybe_unused]] const SDL_Event &ev)
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
                                                                { return endLink(evv, wptr2, krnl); },
                                                                *kernel))
                                                        {
                                                            break;
                                                        }
                                                    }
                                                }
                                            });

                                        if (ev.button.clicks == 2 && !events::containsMouse(*cptr->inNode, ev) && !events::containsMouse(*cptr->outNode, ev))
                                        {
                                            wr.emplace_back(cptr->buildRenderer());
                                        }

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

        std::shared_ptr<Renderer<Rectangle, K>> buildRenderer()
        {
            return Renderer<Rectangle, K>::build({0.0f, 0.0f, 1.0f, 1.0f, std::make_shared<gui::Texture>(512, 512)}, std::shared_ptr(kernel));
        }
    };
}
#endif