#include "Kernel.hh"
#include "Dropzone.hh"
#include "Renderer.hh"

namespace gui
{

    std::vector<std::weak_ptr<Kernel>> Kernel::xKernels;

    void Kernel::executeKernels(cl_uint i)
    {
        for (auto &wptr : xKernels)
        {
            auto sptr = wptr.lock();
            sptr->filter->volume->rFrame = i;
            sptr->execute(sptr->filter->volume, sptr->modified);
        }
    }

    void Kernel::updateLine(float ox, float oy)
    {
        float newX = x + w;
        float newY = outNode->y + outNode->h / 2.0f - outLine.h / 2.0f;
        outLine.angle = std::atan2(oy - newY, ox - newX);
        outLine.update(
            newX - outLine.x,
            newY - outLine.y,
            std::sqrt((oy - outLine.y) * (oy - outLine.y) + (ox - outLine.x) * (ox - outLine.x)) - outLine.w,
            0.0f);
    }

    void Kernel::draw()
    {
        Rectangle::upload();
        title.upload();
        renderButton->draw();
        inNode->draw();
        outNode->draw();
        options->draw();

        if (!outLine.hidden)
        {
            outLine.upload();
        }
    }

    std::shared_ptr<Kernel> Kernel::build(std::shared_ptr<opencl::Filter> &&f, std::shared_ptr<Texture> &&tptr)
    {
        auto sptr = std::shared_ptr<Kernel>(new Kernel(std::move(f), std::forward<std::shared_ptr<Texture>>(tptr)));
        sptr->Rectangle::draw = std::bind(Kernel::draw, sptr.get());
        sptr->Rectangle::resize = std::bind(update, sptr.get(), std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);

        sptr->eventManager->addCallback(
            SDL_MOUSEBUTTONDOWN,
            [wptr = sptr->weak_from_this()](const SDL_Event &e)
            {
                auto ptr = wptr.lock();
                if (events::containsMouse(std::as_const(*ptr->outNode), e))
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
                }
            });

        sptr->eventManager->addCallback(
            SDL_DROPFILE,
            [wptr = sptr->weak_from_this()](const SDL_Event &e)
            {
                auto ptr = wptr.lock();
                if (!ptr->filter->load(e.drop.file))
                {
                    SDL_free(e.drop.file);
                    return;
                }
                
                SDL_free(e.drop.file);

                for (auto itr = xKernels.begin(); itr != xKernels.end(); ++itr)
                {
                    if (itr->lock() == ptr)
                    {
                        xKernels.erase(itr);
                        break;
                    }
                }

                xKernels.push_back(wptr);

                ptr->modified = true;

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

    bool Kernel::endLink(const SDL_Event &e, std::shared_ptr<Kernel> &k)
    {
        if (k.get() == this)
        {
            link = false;
            outLine.hidden = true;
            outLink.reset();
            fire = []([[maybe_unused]] auto &a, [[maybe_unused]] bool b) {};
            return false;
        }

        if (events::containsMouse(*k, e))
        {
            if (k->inLink)
            {
                k->inLink->outLink.reset();
                k->inLink->outLine.hidden = true;
            }

            updateLine(k->inNode->x, k->inNode->y + k->inNode->h / 2.0f);

            outLink = k;
            k->inLink = shared_from_this();

            fire = std::bind(Kernel::execute, k.get(), std::placeholders::_1, std::placeholders::_2);

            outLine.hidden = false;

            // k->active = ptr->active;

            // When executing will allow a save to occur
            k->modified = true;

            return true;
        }
        else
        {
            link = false;
            outLine.hidden = true;
            outLink.reset();
            fire = []([[maybe_unused]] auto &a, [[maybe_unused]] bool b) {};
        }
        return false;
    }

    Kernel::Kernel(std::shared_ptr<opencl::Filter> &&f, std::shared_ptr<Texture> &&tptr) : Rectangle(0.0f, 0.0f, 40.0f, 40.0f), filter(std::forward<std::shared_ptr<opencl::Filter>>(f)), outLine({0.0f, 0.0f, 0.0f, 3.0f}), title(0.0f, 0.0f, static_cast<float>(tptr->textureW), static_cast<float>(tptr->textureH), std::forward<std::shared_ptr<Texture>>(tptr))
    {
        texture->fill({0x5C, 0x5C, 0x5C, 0xFF});

        inNode = (Button::build("IN"));
        outNode = (Button::build("-->"));
        renderButton = (Button::build("+"));

        w = std::max(w, title.w + renderButton->w + 2.0f);
        h = title.h * 3.0f;

        options = filter->getOptions();

        if (options->empty())
        {
            options->hidden = true;
        }
        else
        {
            w = std::max(w, options->w);
        }

        int mx, my;
        SDL_GetMouseState(&mx, &my);

        x = static_cast<float>(mx) - w / 2.0f;
        y = static_cast<float>(my) - h / 2.0f;
        
        if (!options->empty())
        {
            options->resize(x - options->x, y + h - options->y, 0.0f, 0.0f);
            h = h + options->h;
            
            inNode->resize(
                x + 2.0f - inNode->x,
                y + (h - options->h) / 2.0f - inNode->y, 0.0f, 0.0f);
            outNode->resize(
                x + w - 2.0f - outNode->w - outNode->x,
                y + (h - options->h) / 2.0f - outNode->y, 0.0f, 0.0f);
        }
        else
        {
            inNode->resize(
                x + 2.0f - inNode->x,
                y + h / 2.0f - inNode->y, 0.0f, 0.0f);
            outNode->resize(
                x + w - 2.0f - outNode->w - outNode->x,
                y + h / 2.0f - outNode->y, 0.0f, 0.0f);
        }

        title.update(x + w / 2.0f - title.w / 2.0f - renderButton->w / 2.0f - 1.0f - title.x,
                     y - title.y);
        renderButton->resize(
            title.x + title.w + 2.0f - renderButton->x,
            title.y - renderButton->y, 0.0f, 0.0f);

        outLine.hidden = true;
        outLine.texture->fill({0xD3, 0xD3, 0xD3, 0xFF});

        Rectangle::update();

        arm = std::bind(filter->input, std::placeholders::_1);
    }

    void Kernel::execute(std::shared_ptr<data::Volume> &sp, bool m)
    {
        active = true;
        if (m == true)
            modified = m;

        if (sp)
            arm(sp);

        filter->toggle = modified;
        filter->execute();

        if (outLink)
            fire(filter->volume, modified);

        modified = false;
    }

    void Kernel::update(float dx, float dy, float dw, float dh)
    {
        Rectangle::update(dx, dy, dw, dh);
        inNode->resize(dx, dy, dw, dh);
        outNode->resize(dx, dy, dw, dh);
        // outLine.update(dx, dy, dw, dh);
        title.update(dx, dy, dw, dh);
        renderButton->resize(dx, dy, dw, dh);
        options->resize(dx, dy, dw, dh);

        if (outLink)
            updateLine(outLink->inNode->x, outLink->inNode->y + outLink->inNode->h / 2.0f);

        if (inLink)
            inLink->updateLine(inNode->x + inNode->w, inNode->y + inNode->h / 2.0f);
    }

    std::shared_ptr<Renderer> Kernel::buildRenderer(std::vector<std::shared_ptr<Renderer>> &wr)
    {

        if (active)
        {
            try
            {
                filter->volume->buffer.template getInfo<CL_MEM_SIZE>();
                return Renderer::build(wr, {0.0f, 0.0f, 1.0f, 1.0f, std::make_shared<gui::Texture>(512, 512)}, std::shared_ptr(filter->volume), shared_from_this());
            }
            catch (const cl::Error &e)
            {
                SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Render Error", "This kernel does not store a valid volume.", nullptr);
            }
        }
        else
        {
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Render Information", "This kernel is currently inactive. Ultrasound data must be processed by this kernel before it can be displayed.", nullptr);
        }

        return std::shared_ptr<Renderer>(nullptr);
    }

} // namespace gui