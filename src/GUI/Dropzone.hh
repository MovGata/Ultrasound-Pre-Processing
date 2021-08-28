#ifndef GUI_DROPZONE_HH
#define GUI_DROPZONE_HH

#include <unordered_map>
#include <memory>
#include <vector>

#include "Button.hh"
#include "Rectangle.hh"
#include "Texture.hh"
#include "Kernel.hh"

#include "../Events/Concepts.hh"
#include "../Events/EventManager.hh"

#include "../OpenCL/Concepts.hh"

namespace gui
{

    class Dropzone : public Rectangle, public std::enable_shared_from_this<Dropzone>
    {
    private:
        Dropzone(Rectangle &&d) : Rectangle(std::forward<Rectangle>(d))
        {
        }

    public:
        ~Dropzone() = default;
        std::vector<std::shared_ptr<Kernel>> kernels;

        void draw() const
        {
            if (Rectangle::hidden)
                return;

            Rectangle::upload();

            for (auto &&kernel : kernels)
            {
                kernel->draw();
            }
        }

        void erase(std::weak_ptr<Kernel> wptr)
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

        static std::shared_ptr<Dropzone> build(float wWidth, float wHeight)
        {
            auto rptr = std::shared_ptr<Dropzone>(new Dropzone({0.0f, wHeight / 2.0f, wWidth, wHeight / 2.0f}));
            rptr->texture->fill({0x3C, 0x3C, 0x3C, 0xFF});

            rptr->Rectangle::draw = std::bind(Dropzone::draw, rptr.get());
            rptr->Rectangle::resize = std::bind(Rectangle::update, rptr.get(), std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);

            rptr->eventManager->addCallback(
                events::GUI_REDRAW, [wptr = rptr->weak_from_this()](const SDL_Event &e)
                {
                    auto ptr = wptr.lock();

                    // std::pair<float, float> *oldSize = static_cast<std::pair<float, float> *>(e.user.data1);
                    std::pair<float, float> *newSize = static_cast<std::pair<float, float> *>(e.user.data2);

                    // auto xd = ptr->x;
                    // auto yOld = ptr->y;
                    // auto yNew = newSize->second / oldSize->second * (ptr->y + ptr->h) - ptr->h;
                    
                    ptr->resize(
                        0.0f,
                        newSize->second - ptr->h - ptr->y,
                        newSize->first - ptr->x,
                        0.0f
                    );
                    
                    // for (auto &&kernel : ptr->kernels)
                    // {
                    //     kernel->resize(0.0f, yOld, 0.0f, 0.0f);
                    // }
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
    };

}

#endif