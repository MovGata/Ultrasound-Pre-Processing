#include "Kernel.hh"
#include "Dropzone.hh"

namespace gui
{

    KernelBase::KernelBase(std::shared_ptr<Texture> &&tptr) : Rectangle(0.0f, 0.0f, 40.0f, 40.0f), outLine({0.0f, 0.0f, 1.0f, 1.0f}), title(0.0f, 0.0f, static_cast<float>(tptr->textureW), static_cast<float>(tptr->textureH), std::forward<std::shared_ptr<Texture>>(tptr))
    {
    }

    void KernelBase::updateLine(float ox, float oy)
    {
        outLine.y = outNode->y + outNode->h / 2.0f - outLine.h / 2.0f;
        outLine.x = x + w;
        outLine.angle = std::atan2(oy - outLine.y, ox - outLine.x);
        outLine.h = 3.0f;
        outLine.w = std::sqrt((oy - outLine.y) * (oy - outLine.y) + (ox - outLine.x) * (ox - outLine.x));
        outLine.update();
    }

    void KernelBase::draw()
    {
        Rectangle::draw();
        title.draw();
        renderButton->draw();
        inNode->draw();
        outNode->draw();
        options->draw();

        if (!outLine.hidden)
        {
            outLine.draw();
        }
    }

    template <typename K>
    template <typename WK, typename WR, typename D>
    std::shared_ptr<Button<Rectangle>> Kernel<K>::buildButton(const std::string &str, WK &wk, WR &wr, D &d, std::shared_ptr<K> &t)
    {
        auto button = Button<Rectangle>::build(str);

        button->onPress(
            [&sk = wk, &wr, &dropzone = d, k = t, wptr = std::weak_ptr<Texture>(button->texture)]() mutable
            {
                std::shared_ptr<Kernel<K>> ptr = Kernel<K>::build(std::shared_ptr(k), wptr.lock());
                dropzone->kernels.emplace_back(std::make_shared<varType>(ptr));

                ptr->eventManager->addCallback(
                    SDL_MOUSEBUTTONUP,
                    [&dropzone, kwptr = ptr->weak_from_this()](const SDL_Event &e)
                    {
                        auto skptr = kwptr.lock();
                        
                        if (skptr->link)
                        {
                            for (auto &kernel : dropzone->kernels)
                            {
                                if (std::visit(
                                        [&e, wptr2 = skptr->weak_from_this()](auto &&krnl)
                                        { return endLink(e, wptr2, krnl); },
                                        *kernel))
                                {
                                    break;
                                }
                            }
                        }
                        else if (events::containsMouse(*dropzone, e))
                        {
                            skptr->y = std::max(dropzone->y, skptr->y);
                            skptr->y = std::min(dropzone->y + dropzone->h - skptr->h, skptr->y);
                        }
                        else
                        {
                            dropzone->template erase<K>(skptr);
                        }
                    });

                ptr->eventManager->addCallback(
                    SDL_MOUSEBUTTONDOWN,
                    [&sk, kwptr = ptr->weak_from_this()]([[maybe_unused]] const SDL_Event &e)
                    {
                        auto cptr = kwptr.lock();
                        sk.template emplace<std::shared_ptr<Kernel<K>>>(cptr);
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
                sk.template emplace<std::shared_ptr<Kernel<K>>>(std::move(ptr));
            });
        return button;
    }

    template class Kernel<ToPolar>;
    template class Kernel<ToCartesian>;
    template class Kernel<Slice>;
    template class Kernel<Threshold>;
    template class Kernel<Invert>;
    template class Kernel<Clamp>;
    template class Kernel<Contrast>;
    template class Kernel<Log2>;
    template class Kernel<Shrink>;
    template class Kernel<Fade>;
    template class Kernel<Sqrt>;

    template class Kernel<Mindray>;

    KERNELBUTTON(ToPolar);
    KERNELBUTTON(ToCartesian);
    KERNELBUTTON(Slice);
    KERNELBUTTON(Threshold);
    KERNELBUTTON(Invert);
    KERNELBUTTON(Clamp);
    KERNELBUTTON(Contrast);
    KERNELBUTTON(Log2);
    KERNELBUTTON(Shrink);
    KERNELBUTTON(Fade);
    KERNELBUTTON(Sqrt);
    KERNELBUTTON(Mindray);

} // namespace gui