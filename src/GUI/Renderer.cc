#include "Renderer.hh"

namespace gui
{

    Renderer::Renderer(Rectangle &&d, std::shared_ptr<data::Volume> &&ptr, std::shared_ptr<Kernel> &&krnl) : Rectangle(std::forward<Rectangle>(d)), tf(std::forward<std::shared_ptr<data::Volume>>(ptr)), kernel(std::forward<std::shared_ptr<Kernel>>(krnl))
    {
        closeButton = Button::build("x");
        closeButton->resize(x + w - closeButton->w - closeButton->x, y - closeButton->y, 0.0f, 0.0f);

        pauseButton = Button::build("P");
        pauseButton->resize(x - pauseButton->x, y + h - pauseButton->h - pauseButton->y, 0.0f, 0.0f);

        progressBar = Slider::build(x + pauseButton->w + 4.0f, pauseButton->y, w - pauseButton->w - 4.0f, pauseButton->h);

        lastview = glm::mat4(1.0f);
    }

    std::shared_ptr<Renderer> Renderer::build(std::vector<std::shared_ptr<Renderer>> &wr, Rectangle &&d, std::shared_ptr<data::Volume> &&ptr, std::shared_ptr<Kernel> &&krnl)
    {
        auto rptr = std::shared_ptr<Renderer>(new Renderer(std::forward<Rectangle>(d), std::forward<std::shared_ptr<data::Volume>>(ptr), std::forward<std::shared_ptr<Kernel>>(krnl)));
        rptr->Rectangle::draw = std::bind(Renderer::draw, rptr.get());
        rptr->Rectangle::resize = std::bind(update, rptr.get(), std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);

        rptr->modified = true;
        rptr->cFrame = rptr->rFrame = 0;

        rptr->closeButton->onPress(
            [&wr, wptr = rptr->weak_from_this()]() mutable
            {
                auto sptr = wptr.lock();
                for (auto itr = wr.begin(); itr != wr.end(); itr = std::next(itr))
                {
                    if (sptr == *itr)
                    {
                        wr.erase(itr);
                        break;
                    }
                }
            });

        rptr->pauseButton->onPress(
            [&wr, wptr = rptr->weak_from_this()]() mutable
            {
                auto sptr = wptr.lock();
                sptr->paused = !sptr->paused;
            });

        rptr->eventManager->addCallback(
            events::GUI_REDRAW, [wptr = rptr->weak_from_this()](const SDL_Event &e)
            {
                auto sptr = wptr.lock();

                std::pair<float, float> *newSize = static_cast<std::pair<float, float> *>(e.user.data2);

                // Keep the render shaped properly by cutting off the smaller edge.
                sptr->resize(
                    (newSize->first - std::max(newSize->first, newSize->second)) / 2.0f - sptr->x,
                    (newSize->second - std::max(newSize->first, newSize->second)) / 2.0f - sptr->y,
                    std::max(newSize->first, newSize->second) - sptr->w,
                    std::max(newSize->first, newSize->second) - sptr->h);
            });
        rptr->eventManager->addCallback(
            SDL_MOUSEBUTTONDOWN,
            [wptr = rptr->weak_from_this()](const SDL_Event &e)
            {
                auto sptr = wptr.lock();
                if (e.button.button == SDL_BUTTON_LEFT)
                {
                    if (events::containsMouse(std::as_const(*sptr->closeButton), e))
                    {
                        sptr->closeButton->eventManager->process(e);
                        return;
                    }
                    else if (events::containsMouse(std::as_const(*sptr->pauseButton), e))
                    {
                        sptr->pauseButton->eventManager->process(e);
                        return;
                    }
                    else if (events::containsMouse(std::as_const(*sptr->progressBar), e))
                    {
                        sptr->progressBar->eventManager->process(e);
                        sptr->cFrame = static_cast<cl_uint>(std::lerp(0.0f, static_cast<float>(sptr->tf->frames - 1), sptr->progressBar->value));
                        sptr->eventManager->addCallback(
                            SDL_MOUSEMOTION, [wptr](const SDL_Event &ev)
                            {
                                auto ssptr = wptr.lock();
                                ssptr->progressBar->eventManager->process(ev);
                                ssptr->cFrame = static_cast<cl_uint>(std::round(std::lerp(0.0f, static_cast<float>(ssptr->video.size() - 1), ssptr->progressBar->value)));
                            });
                        return;
                    }
                    else
                    {
                        sptr->eventManager->addCallback(
                            SDL_MOUSEMOTION, [wptr](const SDL_Event &ev)
                            {
                                auto ssptr = wptr.lock();
                                ssptr->lastview = glm::rotate(ssptr->lastview, glm::radians(static_cast<float>(ev.motion.yrel)), {1.0f, 0.0f, 0.0f});
                                ssptr->lastview = glm::rotate(ssptr->lastview, -glm::radians(static_cast<float>(ev.motion.xrel)), {0.0f, 1.0f, 0.0f});
                                ssptr->modified = true;
                                ssptr->cFrame = ssptr->rFrame = 0;
                            });
                    }
                }
                else if (e.button.button == SDL_BUTTON_RIGHT)
                {
                    sptr->eventManager->addCallback(
                        SDL_MOUSEMOTION, [wptr](const SDL_Event &ev)
                        {
                            auto ssptr = wptr.lock();
                            events::translate(*ssptr, {-static_cast<float>(4 * ev.motion.xrel) / ssptr->w, -static_cast<float>(4 * ev.motion.yrel) / ssptr->h, 0.0f});
                            ssptr->cFrame = ssptr->rFrame = 0;
                        });
                }
            });
        rptr->eventManager->addCallback(
            SDL_MOUSEBUTTONUP,
            [wptr = rptr->weak_from_this()](const SDL_Event &e)
            {
                auto sptr = wptr.lock();
                sptr->eventManager->clearCallback(SDL_MOUSEMOTION);
                sptr->progressBar->eventManager->process(e);
            });
        rptr->eventManager->addCallback(
            SDL_MOUSEWHEEL,
            [wptr = rptr->weak_from_this()](const SDL_Event &e)
            {
                auto sptr = wptr.lock();
                events::translate(*sptr, {0.0f, 0.0f, e.wheel.y});
                sptr->cFrame = sptr->rFrame = 0;
            });
        return rptr;
    }

    void Renderer::updateView()
    {
        if (!modified)
        {
            return;
        }

        float maxEdge = static_cast<float>(std::max(std::max(tf->depth, tf->length), std::max(tf->depth, tf->width)));

        glm::mat4 model(1.0f);

        // MODEL
        model = glm::scale(model, {maxEdge / static_cast<float>(tf->depth), maxEdge / static_cast<float>(tf->length), maxEdge / static_cast<float>(tf->width)});
        model = glm::rotate(model, glm::radians(90.0f), {0.0f, 0.0f, -1.0f});

        glm::mat4 view(1.0f);

        // VIEW

        view = lastview;
        view = glm::translate(view, {translation.x, translation.y, translation.z});

        // id = glm::translate(id, {0.0f, 0.0f, 2.0f});
        view = model * view;

        GLfloat *modelView = glm::value_ptr(view);

        inv[0] = modelView[0];
        inv[1] = modelView[4];
        inv[2] = modelView[8];
        inv[3] = modelView[12];
        inv[4] = modelView[1];
        inv[5] = modelView[5];
        inv[6] = modelView[9];
        inv[7] = modelView[13];
        inv[8] = modelView[2];
        inv[9] = modelView[6];
        inv[10] = modelView[10];
        inv[11] = modelView[14];
    }

    void Renderer::update(float xx, float yy, float ww, float hh)
    {
        Rectangle::update(xx, yy, ww, hh);
        closeButton->resize(x + w - closeButton->w - closeButton->x, y - closeButton->y, 0.0f, 0.0f);
        pauseButton->resize(x - pauseButton->x, y + h - pauseButton->h - pauseButton->y, 0.0f, 0.0f);
        progressBar->resize(x + pauseButton->w + 4.0f - progressBar->x, pauseButton->y - progressBar->y, w - pauseButton->w - 4.0f - progressBar->w, 0.0f);
    }

    void Renderer::addFrame(GLuint pixelBuffer)
    {
        video.resize(tf->frames);

        if (rFrame >= tf->frames)
            return;

        video[rFrame].resize(512 * 512);

        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pixelBuffer);
        glGetBufferSubData(GL_PIXEL_UNPACK_BUFFER, 0, 512 * 512 * sizeof(GLubyte) * 4, video[rFrame++].data());
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

        // texture->update(pixelBuffer);

        if (rFrame >= tf->frames)
        {
            rFrame = 0;
            modified = false;
        }
    }

    void Renderer::draw()
    {
        cl_uint vFrames = static_cast<cl_uint>(video.size());

        if (vFrames > 1)
            progressBar->modify(static_cast<float>(cFrame) / static_cast<float>(vFrames - 1));

        Uint32 newTick = SDL_GetTicks();

        if (!paused && !video.empty() && (static_cast<float>(newTick - lastTick) > tf->fRate || modified))
        {
            texture->update(video[cFrame++]);
            cFrame = modified ? cFrame % rFrame : cFrame % vFrames;
            lastTick = newTick;
        }

        Rectangle::upload();
        closeButton->draw();
        pauseButton->draw();
        progressBar->draw();
    }

} // namespace gui
