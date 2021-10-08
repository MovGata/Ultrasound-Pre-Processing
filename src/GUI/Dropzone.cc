#include "Dropzone.hh"

namespace
{

}

namespace gui
{

    Dropzone::Dropzone(Rectangle &&d) : Rectangle(std::forward<Rectangle>(d))
    {
    }

    void Dropzone::draw() const
    {
        if (Rectangle::hidden)
            return;

        Rectangle::upload();

        for (auto &&kernel : kernels)
        {
            kernel->draw();
        }
    }

    void Dropzone::erase(std::weak_ptr<Kernel> wptr)
    {
        auto sptr = wptr.lock();
        for (auto itr = kernels.begin(); itr != kernels.end(); itr = std::next(itr))
        {
            if (sptr == *itr)
            {
                if ((*itr)->inLink)
                {
                    (*itr)->inLink->outLink.reset();
                    (*itr)->inLink->outLine.hidden = true;
                }
                kernels.erase(itr);
                break;
            }
        }
    }

    std::shared_ptr<Dropzone> Dropzone::build(float wWidth, float wHeight)
    {
        auto rptr = std::shared_ptr<Dropzone>(new Dropzone({0.0f, wHeight / 2.0f, wWidth, wHeight / 2.0f}));
        rptr->texture->fill({0x3C, 0x3C, 0x3C, 0xFF});

        rptr->Rectangle::draw = std::bind(Dropzone::draw, rptr.get());
        rptr->Rectangle::resize = std::bind(Rectangle::update, rptr.get(), std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);

        rptr->eventManager->addCallback(
            events::GUI_REDRAW, [wptr = rptr->weak_from_this()](const SDL_Event &e)
            {
                std::pair<float, float> *newSize = static_cast<std::pair<float, float> *>(e.user.data2);

                auto ptr = wptr.lock();
                ptr->resize(
                    0.0f,
                    newSize->second - ptr->h - ptr->y,
                    newSize->first - ptr->x,
                    0.0f);
            });

        rptr->eventManager->addCallback(
            SDL_DROPFILE, [wptr = rptr->weak_from_this()](const SDL_Event &e)
            {
                auto ptr = wptr.lock();
                for (auto &kernel : ptr->kernels)
                {
                    if (events::containsMouse(*kernel, e))
                    {
                        kernel->eventManager->process(e);
                        break;
                    }
                }
            });

        rptr->eventManager->addCallback(
            SDL_MOUSEBUTTONDOWN, [wptr = rptr->weak_from_this()](const SDL_Event &e)
            {
                auto ptr = wptr.lock();
                for (auto &kernel : ptr->kernels)
                {
                    if (events::containsMouse(*kernel, e))
                    {
                        kernel->eventManager->process(e);
                        break;
                    }
                }
            });
        return rptr;
    }

    std::shared_ptr<Button> Dropzone::buildKernel(const std::string &str, std::shared_ptr<Kernel> &wk, std::vector<std::shared_ptr<Renderer>> &wr, std::shared_ptr<opencl::Filter> &&f)
    {
        auto button = Button::build(str);

        button->onPress(
            [this, &wk, &wr, filter = std::move(f), wptr = std::weak_ptr<Texture>(button->texture)]() mutable
            {
                std::shared_ptr<Kernel> ptr = Kernel::build(std::shared_ptr(filter), wptr.lock());
                kernels.emplace_back(ptr);

                ptr->eventManager->addCallback(
                    SDL_MOUSEBUTTONUP,
                    [this, kwptr = ptr->weak_from_this()](const SDL_Event &e)
                    {
                        auto skptr = kwptr.lock();

                        if (skptr->link)
                        {   
                            for (auto &kernel : kernels)
                            {
                                if (skptr->endLink(e, kernel))
                                    break;
                            }
                        }
                        else if (events::containsMouse(*this, e))
                        {
                            skptr->y = std::max(y, skptr->y);
                            skptr->y = std::min(y + h - skptr->h, skptr->y);
                        }
                        else
                        {
                            erase(skptr);
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
                                auto rPtr = cptr->buildRenderer(wr);
                                if (rPtr)
                                    wr.emplace_back(std::move(rPtr));
                        }
                    });
                wk = std::move(ptr);
            });
        return button;
    }

} // namespace gui
