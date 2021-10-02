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
    class Dropzone;

    class Kernel : public Rectangle, public std::enable_shared_from_this<Kernel>
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

        std::shared_ptr<Button> inNode;
        std::shared_ptr<Button> outNode;
        std::shared_ptr<Button> renderButton;

        gui::Rectangle outLine;
        gui::Rectangle title;

        std::shared_ptr<Kernel> inLink;
        std::shared_ptr<Kernel> outLink;

        std::shared_ptr<Tree> options;

        bool link = false;


        void updateLine(float ox, float oy);
        void draw();

        static std::shared_ptr<Kernel> build(std::shared_ptr<opencl::Filter> &&f, std::shared_ptr<Texture> &&tptr);        

        ~Kernel() = default;

        void execute(std::shared_ptr<data::Volume> &sp);
        void update(float, float, float, float);

        static bool endLink(const SDL_Event &e, std::weak_ptr<Kernel> wptr, std::shared_ptr<Kernel> &k);

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