#ifndef GUI_DROPZONE_HH
#define GUI_DROPZONE_HH

#include <unordered_map>
#include <memory>
#include <vector>
#include <variant>

#include "Button.hh"
#include "Rectangle.hh"
#include "Texture.hh"
#include "Kernel.hh"

#include "../Events/Concepts.hh"
#include "../Events/EventManager.hh"

#include "../OpenCL/Concepts.hh"

namespace gui
{

    template <concepts::HidableType Drawable>
    class Dropzone : public Drawable, public std::enable_shared_from_this<Dropzone<Drawable>>
    {
    private:
        Dropzone(Drawable &&d) : Drawable(std::forward<Drawable>(d))
        {
        }

    public:
        ~Dropzone() = default;
        std::vector<std::shared_ptr<varType>> kernels;

        events::EventManager eventManager;

        void draw() const
        {
            if (Drawable::hidden)
                return;

            Drawable::draw();

            for (auto &&kernel : kernels)
            {
                std::visit([](auto &&k)
                           { k->draw(); },
                           *kernel);
            }
        }

        template <typename K>
        void erase(std::weak_ptr<Kernel<K>> wptr)
        {
            auto sptr = wptr.lock();
            for (auto itr = kernels.begin(); itr != kernels.end(); itr = std::next(itr))
            {
                if (std::visit(
                        [&sptr](auto &&krnl)
                        {
                            if constexpr (std::same_as<std::decay_t<decltype(krnl)>, std::shared_ptr<Kernel<K>>>)
                            {
                                return sptr == krnl;
                            }

                            return false;
                        },
                        **itr))
                {
                    std::visit(
                        [](auto &&krnl)
                        {
                            if (krnl->inLink)
                            {
                                krnl->inLink->outLink.reset();
                                krnl->inLink->outLine.hidden = true;
                            }
                        },
                        **itr);
                    kernels.erase(itr);
                    break;
                }
            }
        }

        static std::shared_ptr<Dropzone<Drawable>> build(Drawable &&d)
        {
            auto rptr = std::shared_ptr<Dropzone<Drawable>>(new Dropzone<Drawable>(std::forward<Drawable>(d)));
            rptr->eventManager.addCallback(
                events::GUI_REDRAW, [wptr = rptr->weak_from_this()](const SDL_Event &e)
                {
                    auto ptr = wptr.lock();

                    std::pair<float, float> *oldSize = static_cast<std::pair<float, float> *>(e.user.data1);
                    std::pair<float, float> *newSize = static_cast<std::pair<float, float> *>(e.user.data2);

                    // auto xd = ptr->x;
                    auto yd = ptr->y;
                    ptr->w = ptr->w * newSize->first / oldSize->first;
                    ptr->x = ptr->x * newSize->first / oldSize->first;
                    ptr->y = newSize->second / oldSize->second * (ptr->y + ptr->h) - ptr->h;
                    yd = ptr->y - yd;
                    ptr->update();

                    for (auto &&kernel : ptr->kernels)
                    {
                        std::visit(
                            [yd](auto &&k)
                            {
                                k->update(yd);
                            },
                            *kernel);
                    }
                });
            rptr->eventManager.addCallback(
                SDL_MOUSEBUTTONDOWN, [wptr = rptr->weak_from_this()](const SDL_Event &e)
                {
                    auto ptr = wptr.lock();
                    for (auto &kernel : ptr->kernels)
                    {
                        if (std::visit(
                                [e](auto &&k)
                                {
                                    if (events::containsMouse(*k, e))
                                    {
                                        k->eventManager.process(e);
                                        return true;
                                    }
                                    return false;
                                },
                                *kernel))
                        {
                            break;
                        }
                    }
                });
            return rptr;
        }
    };

}

#endif