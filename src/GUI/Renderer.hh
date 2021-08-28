#ifndef GUI_RENDERER_HH
#define GUI_RENDERER_HH

#include <memory>

#include "Button.hh"
#include "Rectangle.hh"

#include "../Data/Volume.hh"

#include "../Events/Concepts.hh"
#include "../Events/EventManager.hh"

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

namespace gui
{
    class Renderer : public Rectangle, public std::enable_shared_from_this<Renderer>
    {
    private:
        std::shared_ptr<Button> closeButton;

        Renderer(Rectangle &&d, std::shared_ptr<data::Volume> &&ptr) : Rectangle(std::forward<Rectangle>(d)), tf(std::forward<std::shared_ptr<data::Volume>>(ptr))
        {
            closeButton = Button::build("x");
            closeButton->resize(x + w - closeButton->w - closeButton->x, y - closeButton->y, 0.0f, 0.0f);
            lastview = glm::mat4(1.0f);
        }

    public:
        ~Renderer() = default;

        std::shared_ptr<data::Volume> tf;

        glm::mat4 lastview;
        glm::vec3 translation = {0.0f, 0.0f, 5.0f};

        bool modified = false;

        std::array<float, 12> inv = {0};

        static std::shared_ptr<Renderer> build(std::vector<std::shared_ptr<Renderer>> &wr, Rectangle &&d, std::shared_ptr<data::Volume> &&ptr)
        {
            auto rptr = std::shared_ptr<Renderer>(new Renderer(std::forward<Rectangle>(d), std::forward<std::shared_ptr<data::Volume>>(ptr)));
            rptr->Rectangle::draw = std::bind(Renderer::draw, rptr.get());
            rptr->Rectangle::resize = std::bind(update, rptr.get(), std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);


            rptr->modified = true;

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
                        std::max(newSize->first, newSize->second) - sptr->h
                    );
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

                        sptr->eventManager->addCallback(
                            SDL_MOUSEMOTION, [wptr](const SDL_Event &ev)
                            {
                                auto ssptr = wptr.lock();
                                ssptr->lastview = glm::rotate(ssptr->lastview, glm::radians(static_cast<float>(ev.motion.yrel)), {1.0f, 0.0f, 0.0f});
                                ssptr->lastview = glm::rotate(ssptr->lastview, -glm::radians(static_cast<float>(ev.motion.xrel)), {0.0f, 1.0f, 0.0f});
                                ssptr->modified = true;
                            });
                    }
                    else if (e.button.button == SDL_BUTTON_RIGHT)
                    {
                        sptr->eventManager->addCallback(
                            SDL_MOUSEMOTION, [wptr](const SDL_Event &ev)
                            {
                                auto ssptr = wptr.lock();
                                events::translate(*ssptr, {-static_cast<float>(4 * ev.motion.xrel) / ssptr->w, -static_cast<float>(4 * ev.motion.yrel) / ssptr->h, 0.0f});
                            });
                        // sptr->eventManager->addCallback(SDL_MOUSEMOTION, events::translateEvent<data::Volume>, *sptr->tf);
                    }
                });
            rptr->eventManager->addCallback(
                SDL_MOUSEBUTTONUP,
                [wptr = rptr->weak_from_this()]([[maybe_unused]] const SDL_Event &)
                {
                    auto sptr = wptr.lock();
                    sptr->eventManager->clearCallback(SDL_MOUSEMOTION);
                });
            rptr->eventManager->addCallback(
                SDL_MOUSEWHEEL,
                [wptr = rptr->weak_from_this()](const SDL_Event &e)
                {
                    auto sptr = wptr.lock();
                    events::translate(*sptr, {0.0f, 0.0f, e.wheel.y});
                });
            return rptr;
        }

        void updateView()
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

        void update(float xx = 0.0f, float yy = 0.0f, float ww = 0.0f, float hh = 0.0f)
        {
            Rectangle::update(xx, yy, ww, hh);
            closeButton->resize(x + w - closeButton->w - closeButton->x, y - closeButton->y, 0.0f, 0.0f);
        }

        void draw()
        {
            Rectangle::upload();
            closeButton->draw();
        }
    };

}

#endif