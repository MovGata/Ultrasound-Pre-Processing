#ifndef GUI_KERNEL_HH
#define GUI_KERNEL_HH

#include <functional>
#include <string>
#include <memory>
#include <vector>

#include <glm/ext.hpp>

#include "Rectangle.hh"
#include "Button.hh"
#include "Texture.hh"
#include "Renderer.hh"
#include "Tree.hh"
#include "Slider.hh"

#include "../OpenCL/Kernel.hh"
#include "../OpenCL/Filter.hh"
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

    using TreeType = gui::Tree<gui::Button<gui::Rectangle>, std::tuple<gui::Button<gui::Rectangle>>, std::tuple<gui::Slider>>;
}

namespace gui
{

    class KernelBase : public Rectangle
    {
    public:
        std::shared_ptr<Button<Rectangle>> inNode;
        std::shared_ptr<Button<Rectangle>> outNode;
        std::shared_ptr<Button<Rectangle>> renderButton;

        gui::Rectangle outLine;
        gui::Rectangle title;

        std::shared_ptr<KernelBase> inLink;
        std::shared_ptr<KernelBase> outLink;

        std::shared_ptr<TreeType> options;

        void updateLine(float ox, float oy);
        void draw();

    protected:
        KernelBase(std::shared_ptr<Texture> &&tptr);
        ~KernelBase() = default;
    };

    template <concepts::HidableType Drawable>
    class Dropzone;

    class Kernel : public KernelBase, public std::enable_shared_from_this<Kernel>
    {
    private:
        Kernel(std::shared_ptr<opencl::Filter> &&f, std::shared_ptr<Texture> &&tptr);

        bool move = true;
        std::weak_ptr<events::EventManager> optionEvent;

    public:
        std::shared_ptr<opencl::Filter> filter;
        std::function<void(void)> arm, fire;

        std::shared_ptr<events::EventManager> eventManager;

        bool link = false;

        static std::shared_ptr<Kernel> build(std::shared_ptr<opencl::Filter> &&f, std::shared_ptr<Texture> &&tptr)
        {
            auto sptr = std::shared_ptr<Kernel>(new Kernel(std::forward<std::shared_ptr<opencl::Filter>>(f), std::forward<std::shared_ptr<Texture>>(tptr)));
            sptr->eventManager->addCallback(
                SDL_MOUSEBUTTONDOWN,
                [wptr = sptr->weak_from_this()](const SDL_Event &e)
                {
                    auto ptr = wptr.lock();
                    if (events::containsMouse(std::as_const(*ptr->inNode), e))
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
                            ptr->outLink->inLink.reset();
                        }

                        ptr->updateLine(static_cast<float>(e.motion.x), static_cast<float>(e.motion.y));
                        ptr->outLine.hidden = false;
                        ptr->link = true;
                        ptr->outNode->eventManager->process(e);
                        return;
                    }
                    else if (events::containsMouse(std::as_const(*ptr->options), e))
                    {
                        ptr->h = ptr->h - ptr->options->h;
                        ptr->options->eventManager->process(e);
                        ptr->optionEvent = ptr->options->eventManager;
                        ptr->h = ptr->h + ptr->options->h;
                        ptr->Rectangle::update();
                    }
                    else
                    {
                        ptr->link = false;
                        ptr->move = true;
                        if (e.button.clicks == 2)
                        {
                        }
                    }
                });

            sptr->eventManager->addCallback(
                SDL_MOUSEBUTTONUP,
                [wptr = sptr->weak_from_this()](const SDL_Event &e)
                {
                    auto ptr = wptr.lock();
                    ptr->move = false;
                    if (!ptr)
                        return;

                    auto optr = ptr->optionEvent.lock();
                    if (optr)
                    {
                        optr->process(e);
                        ptr->optionEvent.reset();
                    }
                    else if (events::containsMouse(std::as_const(*ptr->renderButton), e))
                    {
                        ptr->renderButton->eventManager->process(e);
                        return;
                    }
                });

            sptr->eventManager->addCallback(
                SDL_MOUSEMOTION,
                [wptr = sptr->weak_from_this()](const SDL_Event &e)
                {
                    auto ptr = wptr.lock();
                    auto optr = ptr->optionEvent.lock();

                    if (ptr->link)
                    {
                        ptr->updateLine(static_cast<float>(e.motion.x), static_cast<float>(e.motion.y));
                    }
                    else if (optr)
                    {
                        optr->process(e);
                    }
                    else if (ptr->move)
                    {
                        auto dy = ptr->y;
                        auto dx = ptr->x;

                        ptr->Rectangle::update(static_cast<float>(e.motion.x) - ptr->w / 2.0f, static_cast<float>(e.motion.y) - ptr->h / 2.0f);

                        dy = ptr->y - dy;
                        dx = ptr->x - dx;

                        ptr->inNode->update(ptr->inNode->x + dx, ptr->inNode->y + dy);
                        ptr->outNode->update(ptr->outNode->x + dx, ptr->outNode->y + dy);
                        ptr->title.update(ptr->title.x + dx, ptr->title.y + dy);
                        ptr->renderButton->update(ptr->renderButton->x + dx, ptr->renderButton->y + dy);
                        ptr->options->update(dx, dy);

                        if (ptr->outLink)
                            ptr->updateLine(ptr->outLink->inNode->x, ptr->outLink->inNode->y + ptr->outLink->inNode->h / 2.0f);

                        if (ptr->inLink)
                            ptr->inLink->updateLine(ptr->inNode->x + ptr->inNode->w, ptr->inNode->y + ptr->inNode->h / 2.0f);
                    }
                });
            return sptr;
        }

        ~Kernel() = default;

        void execute();
        void update(float dy);

        static bool endLink(const SDL_Event &e, std::weak_ptr<Kernel> wptr, std::shared_ptr<Kernel> &k)
        {
            auto ptr = wptr.lock();

            if (k.get() == ptr.get())
            {
                ptr->link = false;
                ptr->outLine.hidden = true;
                return false;
            }

            if (events::containsMouse(*k, e))
            {
                if (k->inLink)
                {
                    k->inLink->outLink.reset();
                    k->inLink->outLine.hidden = true;
                }

                ptr->updateLine(k->inNode->x, k->inNode->y + k->inNode->h / 2.0f);

                ptr->outLink = std::static_pointer_cast<KernelBase>(k);
                k->inLink = std::static_pointer_cast<KernelBase>(ptr);

                ptr->fire = std::bind(Kernel::execute, k.get());
                k->arm = std::bind(k->filter->input, ptr->filter->volume);

                ptr->outLine.hidden = false;
                return true;
            }
            else
            {
                ptr->link = false;
                ptr->outLine.hidden = true;
            }
            return false;
        }

        template <typename D>
        static std::shared_ptr<Button<Rectangle>> buildButton(const std::string &str, std::shared_ptr<Kernel> &wk, std::vector<std::shared_ptr<Renderer<Rectangle>>> &wr, D &d, std::shared_ptr<opencl::Filter> &&f)
        {
            auto button = Button<Rectangle>::build(str);

            button->onPress(
                [&wk, &wr, &dropzone = d, filter = std::move(f), wptr = std::weak_ptr<Texture>(button->texture)]() mutable
                {
                    std::shared_ptr<Kernel> ptr = Kernel::build(std::shared_ptr(filter), wptr.lock());
                    dropzone->kernels.emplace_back(ptr);

                    ptr->eventManager->addCallback(
                        SDL_MOUSEBUTTONUP,
                        [&dropzone, kwptr = ptr->weak_from_this()](const SDL_Event &e)
                        {
                            auto skptr = kwptr.lock();

                            if (skptr->link)
                            {
                                for (auto &kernel : dropzone->kernels)
                                {
                                    if (endLink(e, skptr->weak_from_this(), kernel))
                                        break;
                                }
                            }
                            else if (events::containsMouse(*dropzone, e))
                            {
                                skptr->y = std::max(dropzone->y, skptr->y);
                                skptr->y = std::min(dropzone->y + dropzone->h - skptr->h, skptr->y);
                            }
                            else
                            {
                                dropzone->erase(skptr);
                            }
                        });

                    ptr->eventManager->addCallback(
                        SDL_MOUSEBUTTONDOWN,
                        [&wk, kwptr = ptr->weak_from_this()]([[maybe_unused]] const SDL_Event &e)
                        {
                            wk = kwptr.lock();
                        });

                    ptr->renderButton->onRelease(
                        [kwptr = ptr->weak_from_this(), &wr](const SDL_Event &ev)
                        {
                            auto cptr = kwptr.lock();
                            if (!events::containsMouse(*cptr->inNode, ev) && !events::containsMouse(*cptr->outNode, ev))
                            {
                                wr.emplace_back(cptr->buildRenderer(wr));
                            }
                        });
                    wk = std::move(ptr);
                });
            return button;
        }

        std::shared_ptr<Renderer<Rectangle>> buildRenderer(std::vector<std::shared_ptr<Renderer<Rectangle>>> &wr);
    };

}
#endif