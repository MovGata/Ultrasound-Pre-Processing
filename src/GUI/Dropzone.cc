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

} // namespace gui
