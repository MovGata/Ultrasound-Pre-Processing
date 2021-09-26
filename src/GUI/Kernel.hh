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
// #include "Renderer.hh"
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
}

namespace gui
{

    class Renderer;

    class KernelBase : public Rectangle
    {
    public:
        std::shared_ptr<Button> inNode;
        std::shared_ptr<Button> outNode;
        std::shared_ptr<Button> renderButton;

        gui::Rectangle outLine;
        gui::Rectangle title;

        std::shared_ptr<KernelBase> inLink;
        std::shared_ptr<KernelBase> outLink;

        std::shared_ptr<Tree> options;

        void updateLine(float ox, float oy);
        void draw();

    protected:
        KernelBase(std::shared_ptr<Texture> &&tptr);
        ~KernelBase() = default;
    };

    class Dropzone;

    class Kernel : public KernelBase, public std::enable_shared_from_this<Kernel>
    {
    private:
        Kernel(std::shared_ptr<opencl::Filter> &&f, std::shared_ptr<Texture> &&tptr);

        bool move = true;
        std::weak_ptr<events::EventManager> optionEvent;
        bool active = false;

    public:
        std::shared_ptr<opencl::Filter> filter;
        std::function<void(std::shared_ptr<data::Volume> &)> fire;
        std::function<void(std::shared_ptr<data::Volume> &)> arm;
        static std::vector<std::weak_ptr<Kernel>> xKernels;

        bool link = false;

        static std::shared_ptr<Kernel> build(std::shared_ptr<opencl::Filter> &&f, std::shared_ptr<Texture> &&tptr)
        {
            auto sptr = std::shared_ptr<Kernel>(new Kernel(std::move(f), std::forward<std::shared_ptr<Texture>>(tptr)));
            sptr->Rectangle::draw = std::bind(Kernel::draw, sptr.get());
            sptr->Rectangle::resize = std::bind(update, sptr.get(), std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);

            sptr->eventManager->addCallback(
                SDL_MOUSEBUTTONDOWN,
                [wptr = sptr->weak_from_this()](const SDL_Event &e)
                {
                    auto ptr = wptr.lock();
                    if (events::containsMouse(std::as_const(*ptr->inNode), e))
                    {
                        // if (e.button.clicks == 2)
                        // {
                            // xKernels.push_back(wptr);
                            //ptr->execute(ptr->filter->volume); // Store function in main loop and add counter to volume
                        // }
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
                        ptr->optionEvent = ptr->options->subManager;
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
                SDL_DROPFILE,
                [wptr = sptr->weak_from_this()](const SDL_Event &e)
                {
                    auto ptr = wptr.lock();
                    std::cout << e.drop.file << std::endl;
                    ptr->filter->load(e.drop.file);
                    SDL_free(e.drop.file);
                    xKernels.push_back(wptr);
                    executeKernels(0);
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
                        ptr->resize(static_cast<float>(e.motion.xrel), static_cast<float>(e.motion.yrel), 0.0f, 0.0f);
                    }
                });
            return sptr;
        }

        ~Kernel() = default;

        void execute(std::shared_ptr<data::Volume> &sp);
        void update(float, float, float, float);

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

                ptr->fire = std::bind(Kernel::execute, k.get(), std::placeholders::_1);

                ptr->outLine.hidden = false;

                k->active = ptr->active;

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
        static std::shared_ptr<Button> buildButton(const std::string &str, std::shared_ptr<Kernel> &wk, std::vector<std::shared_ptr<Renderer>> &wr, D &d, std::shared_ptr<opencl::Filter> &&f)
        {
            auto button = Button::build(str);

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
                                if (cptr->active)
                                    wr.emplace_back(cptr->buildRenderer(wr));
                            }
                        });
                    wk = std::move(ptr);
                });
            return button;
        }

        std::shared_ptr<Renderer> buildRenderer(std::vector<std::shared_ptr<Renderer>> &wr);

        static void executeKernels(cl_uint i);

    };

}
#endif