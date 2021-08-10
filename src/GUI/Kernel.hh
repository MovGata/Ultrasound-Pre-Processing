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

#define KERNELBUTTON(K) template std::shared_ptr<Button<Rectangle>> Kernel<K>::buildButton<gui::varType, std::vector<gui::renType>, std::shared_ptr<gui::Dropzone<gui::Rectangle>>>(const std::string &str, gui::varType &wk, std::vector<gui::renType> &wr, std::shared_ptr<gui::Dropzone<gui::Rectangle>> &d, std::shared_ptr<K> &t)

namespace
{
    using namespace opencl;
    using namespace ultrasound;
}

namespace gui
{

    template <concepts::HidableType Drawable>
    class Dropzone;

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

            renderButton = (Button<Rectangle>::build("+"));

            w = std::max(w, title.w + renderButton->w + 2.0f);
            h = title.h * 3.0f;

            int mx, my;
            SDL_GetMouseState(&mx, &my);

            x = static_cast<float>(mx) - w / 2.0f;
            y = static_cast<float>(my) - h / 2.0f;

            title.x = x + w / 2.0f - title.w / 2.0f - renderButton->w / 2.0f - 1.0f;
            title.y = y;
            title.update();

            renderButton->x = title.x + title.w + 2.0f;
            renderButton->y = title.y;
            renderButton->update();

            inNode->y = y + h / 2.0f;
            inNode->x = x + 2.0f;
            inNode->update();

            outNode->y = y + h / 2.0f;
            outNode->x = x + w - 2.0f - outNode->w;
            outNode->update();

            outLine.hidden = true;
            outLine.texture->fill({0xD3, 0xD3, 0xD3, 0xFF});

            Rectangle::update();
        }

    public:
        std::shared_ptr<K> kernel;
        std::shared_ptr<Button<Rectangle>> inNode;
        std::shared_ptr<Button<Rectangle>> outNode;
        std::shared_ptr<Button<Rectangle>> renderButton;

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

                        ptr->updateLine(static_cast<float>(e.motion.x), static_cast<float>(e.motion.y));
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
                SDL_MOUSEBUTTONUP,
                [wptr = sptr->weak_from_this()](const SDL_Event &e)
                {
                    auto ptr = wptr.lock();
                    if (!ptr)
                        return;
                    if (events::containsMouse(std::as_const(*ptr->renderButton), e))
                    {
                        ptr->renderButton->eventManager.process(e);
                        return;
                    }
                });

            sptr->eventManager.addCallback(
                SDL_MOUSEMOTION,
                [wptr = sptr->weak_from_this()](const SDL_Event &e)
                {
                    auto ptr = wptr.lock();

                    if (ptr->link)
                    {
                        ptr->updateLine(static_cast<float>(e.motion.x), static_cast<float>(e.motion.y));
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

                        ptr->renderButton->y += dy;
                        ptr->renderButton->x += dx;
                        ptr->renderButton->update();

                        if (ptr->outLink)
                        {
                            float oy = std::visit([](auto &&l)
                                                  { return l->inNode->y + l->inNode->h / 2.0f; },
                                                  *ptr->outLink);
                            float ox = std::visit([](auto &&l)
                                                  { return l->inNode->x; },
                                                  *ptr->outLink);

                            ptr->updateLine(ox, oy);
                        }

                        if (ptr->inLink)
                        {
                            std::visit([oy = ptr->inNode->y + ptr->inNode->h / 2.0f, ox = ptr->inNode->x + ptr->inNode->w](auto &&l)
                                       { l->updateLine(ox, oy); },
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
            renderButton->draw();
            inNode->draw();
            outNode->draw();

            if (!outLine.hidden)
            {
                outLine.draw();
            }
        }

        void updateLine(float ox, float oy)
        {
            outLine.y = outNode->y + outNode->h / 2.0f - outLine.h / 2.0f;
            outLine.x = x + w;
            outLine.angle = std::atan2(oy - outLine.y, ox - outLine.x);
            outLine.h = 3.0f;
            outLine.w = std::sqrt((oy - outLine.y) * (oy - outLine.y) + (ox - outLine.x) * (ox - outLine.x));
            outLine.update();
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
            renderButton->y += dy;

            inNode->update();
            outNode->update();
            outLine.update();
            title.update();
            renderButton->update();
        }

        template <typename KT>
        static bool endLink(const SDL_Event &e, std::weak_ptr<Kernel<K>> wptr, std::shared_ptr<KT> &k)
        {
            auto ptr = wptr.lock();
            if constexpr (std::same_as<std::decay_t<decltype(k)>, std::shared_ptr<Kernel<K>>>)
            {
                if (k.get() == ptr.get())
                {
                    ptr->link = false;
                    ptr->outLine.hidden = true;
                    return false;
                }
            }

            if (ptr->kernel->out == k->kernel->in && events::containsMouse(*k, e))
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

                ptr->updateLine(k->inNode->x, k->inNode->y + k->inNode->h / 2.0f);
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
            return false;
        }

        template <typename WK, typename WR, typename D>
        static std::shared_ptr<Button<Rectangle>> buildButton(const std::string &str, WK &wk, WR &wr, D &d, std::shared_ptr<K> &t);

        std::shared_ptr<Renderer<Rectangle, K>> buildRenderer()
        {
            return Renderer<Rectangle, K>::build({0.0f, 0.0f, 1.0f, 1.0f, std::make_shared<gui::Texture>(512, 512)}, std::shared_ptr(kernel));
        }
    };

    extern template class Kernel<opencl::ToPolar>;
    extern template class Kernel<opencl::ToCartesian>;
    extern template class Kernel<opencl::Clamp>;
    extern template class Kernel<opencl::Contrast>;
    extern template class Kernel<opencl::Fade>;
    extern template class Kernel<opencl::Invert>;
    extern template class Kernel<opencl::Log2>;
    extern template class Kernel<opencl::Shrink>;
    extern template class Kernel<opencl::Slice>;
    extern template class Kernel<opencl::Sqrt>;
    extern template class Kernel<opencl::Threshold>;
    extern template class Kernel<ultrasound::Mindray>;

    extern KERNELBUTTON(ToPolar);
    extern KERNELBUTTON(ToCartesian);
    extern KERNELBUTTON(Slice);
    extern KERNELBUTTON(Threshold);
    extern KERNELBUTTON(Invert);
    extern KERNELBUTTON(Clamp);
    extern KERNELBUTTON(Contrast);
    extern KERNELBUTTON(Log2);
    extern KERNELBUTTON(Shrink);
    extern KERNELBUTTON(Fade);
    extern KERNELBUTTON(Sqrt);
    extern KERNELBUTTON(Mindray);

}
#endif