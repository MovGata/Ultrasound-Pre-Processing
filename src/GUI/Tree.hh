#ifndef GUI_TREE_HH
#define GUI_TREE_HH

#include <memory>
#include <variant>
#include <vector>

#include <SDL2/SDL_events.h>

#include "MultiTexture.hh"
#include "Rectangle.hh"
#include "Button.hh"

#include "../Events/Concepts.hh"
#include "../Events/GUI.hh"
#include "../Events/EventManager.hh"

namespace gui
{

    template <concepts::PositionableType Trunk, typename Branches, typename Leaves>
    class Tree;

    template <concepts::PositionableType Trunk, concepts::PositionableType... Branches, concepts::PositionableType... Leaves>
    requires concepts::ProcessorType<Trunk> && concepts::TranslatableType<Trunk> &&(concepts::ProcessorType<Branches> &&...) && (concepts::TranslatableType<Branches> && ...) && (concepts::ProcessorType<Leaves> && ...) && (concepts::TranslatableType<Leaves> && ...) //
        class Tree<Trunk, std::tuple<Branches...>, std::tuple<Leaves...>> : public Rectangle,
        public std::enable_shared_from_this<Tree<Trunk, std::tuple<Branches...>, std::tuple<Leaves...>>>
    {
    private:
        std::vector<std::variant<std::shared_ptr<Tree<Branches, std::tuple<Branches...>, std::tuple<Leaves...>>>...>> branches;
        std::vector<std::variant<std::shared_ptr<Leaves>...>> leaves;

        MultiTexture<Trunk, 2> trunk;

        bool open = false;

    protected:
        Tree(Trunk &&t);
        Tree(Trunk &&t, std::shared_ptr<Texture> &&alt);

    public:
        std::shared_ptr<events::EventManager> eventManager;
        std::weak_ptr<events::EventManager> subManager;

        static std::shared_ptr<Tree<Trunk, std::tuple<Branches...>, std::tuple<Leaves...>>> build(const std::string &str);
        static std::shared_ptr<Tree<Trunk, std::tuple<Branches...>, std::tuple<Leaves...>>> build(Trunk &&t);
        static std::shared_ptr<Tree<Trunk, std::tuple<Branches...>, std::tuple<Leaves...>>> build(Trunk &&t, std::shared_ptr<Texture> &&alt);

        Tree() = default;
        ~Tree() = default;

        void draw();

        float toggle();
        void update();
        void update(float);
        void update(float, float);

        template <concepts::PositionableType U>
        requires concepts::ProcessorType<U> && concepts::TranslatableType<U>
        void addBranch(std::shared_ptr<U> &&u, float offset = 0.0f);

        template <concepts::PositionableType U>
        requires concepts::ProcessorType<U> && concepts::TranslatableType<U>
        void addLeaf(std::shared_ptr<U> &&u, float offset = 0.0f);
    };

    template <concepts::PositionableType Trunk, concepts::PositionableType... Branches, concepts::PositionableType... Leaves>
    requires concepts::ProcessorType<Trunk> && concepts::TranslatableType<Trunk> &&(concepts::ProcessorType<Branches> &&...) && (concepts::TranslatableType<Branches> && ...) && (concepts::ProcessorType<Leaves> && ...) && (concepts::TranslatableType<Leaves> && ...) //
        Tree<Trunk, std::tuple<Branches...>, std::tuple<Leaves...>>::Tree(Trunk &&t) : trunk(std::forward<Trunk>(t)),
    eventManager(std::make_shared<events::EventManager>())
    {
        trunk.setTexture(0, std::shared_ptr<Texture>(t.texture));
        trunk.setTexture(1, std::shared_ptr<Texture>(t.texture));

        w = trunk.w;
        h = trunk.h;
        update();
    }

    template <concepts::PositionableType Trunk, concepts::PositionableType... Branches, concepts::PositionableType... Leaves>
    requires concepts::ProcessorType<Trunk> && concepts::TranslatableType<Trunk> &&(concepts::ProcessorType<Branches> &&...) && (concepts::TranslatableType<Branches> && ...) && (concepts::ProcessorType<Leaves> && ...) && (concepts::TranslatableType<Leaves> && ...) //
        Tree<Trunk, std::tuple<Branches...>, std::tuple<Leaves...>>::Tree(Trunk &&t, std::shared_ptr<Texture> &&alt) : trunk(std::forward<Trunk>(t)),
    eventManager(std::make_shared<events::EventManager>())
    {
        trunk.setTexture(0, std::shared_ptr<Texture>(t.texture));
        trunk.setTexture(1, std::forward<std::shared_ptr<Texture>>(alt));

        w = trunk.w;
        h = trunk.h;
        update();
    }

    template <concepts::PositionableType Trunk, concepts::PositionableType... Branches, concepts::PositionableType... Leaves>
    requires concepts::ProcessorType<Trunk> && concepts::TranslatableType<Trunk> &&(concepts::ProcessorType<Branches> &&...) && (concepts::TranslatableType<Branches> && ...) && (concepts::ProcessorType<Leaves> && ...) && (concepts::TranslatableType<Leaves> && ...) std::shared_ptr<Tree<Trunk, std::tuple<Branches...>, std::tuple<Leaves...>>> //
        Tree<Trunk, std::tuple<Branches...>, std::tuple<Leaves...>>::build(const std::string &str)
    {
        int w, h;
        TTF_SizeText(Texture::lastFont, str.c_str(), &w, &h);
        auto t = std::make_shared<gui::Texture>(w + 2, h + 2);
        t->addText(Texture::lastFont, str.c_str());

        auto rec = gui::Button<gui::Rectangle>::build({0.0f, 0.0f, static_cast<float>(w) + 2.0f, static_cast<float>(h) + 2.0f, std::move(t)});
        rec->update();

        return build(gui::Button<gui::Rectangle>({*rec}));
    }

    template <concepts::PositionableType Trunk, concepts::PositionableType... Branches, concepts::PositionableType... Leaves>
    requires concepts::ProcessorType<Trunk> && concepts::TranslatableType<Trunk> &&(concepts::ProcessorType<Branches> &&...) && (concepts::TranslatableType<Branches> && ...) && (concepts::ProcessorType<Leaves> && ...) && (concepts::TranslatableType<Leaves> && ...) std::shared_ptr<Tree<Trunk, std::tuple<Branches...>, std::tuple<Leaves...>>> //
        Tree<Trunk, std::tuple<Branches...>, std::tuple<Leaves...>>::build(Trunk &&t)
    {
        auto rptr = std::shared_ptr<Tree<Trunk, std::tuple<Branches...>, std::tuple<Leaves...>>>(new Tree<Trunk, std::tuple<Branches...>, std::tuple<Leaves...>>(std::forward<Trunk>(t)));

        rptr->eventManager->addCallback(
            events::GUI_REDRAW, [wptr = rptr->weak_from_this()](const SDL_Event &e)
            {
                auto ptr = wptr.lock();

                std::pair<float, float> *oldSize = static_cast<std::pair<float, float> *>(e.user.data1);
                std::pair<float, float> *newSize = static_cast<std::pair<float, float> *>(e.user.data2);

                auto xd = newSize->first / oldSize->first * (ptr->x + ptr->w) - ptr->w;
                auto yd = ptr->y * newSize->second / oldSize->second;
                // ptr->h = ptr->h * newSize->second / oldSize->second;

                for (auto &&branch : ptr->branches)
                {
                    std::visit(
                        [offsetY = yd - ptr->y, offsetX = xd - ptr->x](auto &&b)
                        {
                            b->update(b->x + offsetX, b->y + offsetY);
                        },
                        branch);
                }

                for (auto &&leaf : ptr->leaves)
                {
                    std::visit(
                        [offsetY = yd - ptr->y, offsetX = xd - ptr->x](auto &&l)
                        {
                            l->update(l->x + offsetX, l->y + offsetY);
                        },
                        leaf);
                }

                ptr->update(xd, yd);
                ptr->trunk.update(xd, yd);
            });
        rptr->eventManager->addCallback(
            SDL_MOUSEBUTTONDOWN, [wptr = rptr->weak_from_this()](const SDL_Event &e)
            {
                auto ptr = wptr.lock();

                float offset = 0;
                if (events::containsMouse(std::as_const(ptr->trunk), e))
                {
                    ptr->toggle();
                }
                else
                {
                    bool branchHit = false;
                    bool leafHit = false;
                    for (auto &&branch : ptr->branches)
                    {
                        if (!branchHit && !leafHit)
                        {
                            std::visit(
                                [e, &offset, &branchHit, &leafHit, &ptr](auto &&b)
                                {
                                    if (events::containsMouse(std::as_const(b->trunk), e))
                                    {
                                        offset = -b->h; // Subtract size before closing/opening
                                        b->eventManager->process(e);
                                        ptr->subManager = b->eventManager;
                                        offset += b->h; // Add size after opening/closing
                                        branchHit = true;
                                    }
                                    else if (events::containsMouse(std::as_const(*b), e))
                                    {
                                        b->eventManager->process(e);
                                        ptr->subManager = b->eventManager;
                                        leafHit = true;
                                    }
                                },
                                branch);
                        }
                        else
                        {
                            std::visit(
                                [offset](auto &&b)
                                {
                                    b->update(offset);
                                },
                                branch);
                        }
                    }

                    if (branchHit)
                    {
                        ptr->h += offset;
                        ptr->update();
                        for (auto &&leaf : ptr->leaves)
                        {
                            std::visit(
                                [offset](auto &&l)
                                {
                                    l->update(l->x, l->y + offset);
                                },
                                leaf);
                        }
                    }
                    else if (!leafHit)
                    {
                        for (auto &&leaf : ptr->leaves)
                        {
                            if (std::visit(
                                    [&e, &branchHit, &ptr](auto &&l)
                                    {
                                        if (events::containsMouse(std::as_const(*l), e))
                                        {
                                            l->eventManager->process(e);
                                            ptr->subManager = l->eventManager;
                                            return true;
                                        }
                                        else
                                        {
                                            return false;
                                        }
                                    },
                                    leaf))
                            {
                                break;
                            }
                        }
                    }
                }
            });
        rptr->eventManager->addCallback(
            SDL_MOUSEBUTTONUP, [wptr = rptr->weak_from_this()]([[maybe_unused]] const SDL_Event &e)
            {
                auto ptr = wptr.lock();
                ptr->subManager.reset();
            });
        return rptr;
    }

    template <concepts::PositionableType Trunk, concepts::PositionableType... Branches, concepts::PositionableType... Leaves>
    requires concepts::ProcessorType<Trunk> && concepts::TranslatableType<Trunk> &&(concepts::ProcessorType<Branches> &&...) && (concepts::TranslatableType<Branches> && ...) && (concepts::ProcessorType<Leaves> && ...) && (concepts::TranslatableType<Leaves> && ...) std::shared_ptr<Tree<Trunk, std::tuple<Branches...>, std::tuple<Leaves...>>> //
        Tree<Trunk, std::tuple<Branches...>, std::tuple<Leaves...>>::build(Trunk &&t, std::shared_ptr<Texture> &&alt)
    {
        auto rptr = std::shared_ptr<Tree<Trunk, std::tuple<Branches...>, std::tuple<Leaves...>>>(new Tree<Trunk, std::tuple<Branches...>, std::tuple<Leaves...>>(std::forward<Trunk>(t), std::forward<std::shared_ptr<Texture>>(alt)));

        rptr->eventManager->addCallback(
            events::GUI_REDRAW, [wptr = rptr->weak_from_this()](const SDL_Event &e)
            {
                auto ptr = wptr.lock();

                std::pair<float, float> *oldSize = static_cast<std::pair<float, float> *>(e.user.data1);
                std::pair<float, float> *newSize = static_cast<std::pair<float, float> *>(e.user.data2);

                auto xd = newSize->first / oldSize->first * (ptr->x + ptr->w) - ptr->w;
                auto yd = ptr->y * newSize->second / oldSize->second;
                // ptr->h = ptr->h * newSize->second / oldSize->second;

                for (auto &&branch : ptr->branches)
                {
                    std::visit(
                        [offsetY = yd - ptr->y, offsetX = xd - ptr->x](auto &&b)
                        {
                            b->update(b->x + offsetX, b->y + offsetY);
                        },
                        branch);
                }

                for (auto &&leaf : ptr->leaves)
                {
                    std::visit(
                        [offsetY = yd - ptr->y, offsetX = xd - ptr->x](auto &&l)
                        {
                            l->update(l->x + offsetX, l->y + offsetY);
                        },
                        leaf);
                }

                ptr->update(xd, yd);
                ptr->trunk.update(xd, yd);
            });
        rptr->eventManager->addCallback(
            SDL_MOUSEBUTTONDOWN, [wptr = rptr->weak_from_this()](const SDL_Event &e)
            {
                auto ptr = wptr.lock();

                float offset = 0;
                if (events::containsMouse(std::as_const(ptr->trunk), e))
                {
                    ptr->toggle();
                }
                else
                {
                    bool branchHit = false;
                    bool leafHit = false;
                    for (auto &&branch : ptr->branches)
                    {
                        if (!branchHit && !leafHit)
                        {
                            std::visit(
                                [e, &offset, &branchHit, &leafHit](auto &&b)
                                {
                                    if (events::containsMouse(std::as_const(b->trunk), e))
                                    {
                                        offset = -b->h; // Subtract size before closing/opening
                                        b->eventManager->process(e);
                                        offset += b->h; // Add size after opening/closing
                                        branchHit = true;
                                    }
                                    else if (events::containsMouse(std::as_const(*b), e))
                                    {
                                        b->eventManager->process(e);
                                        leafHit = true;
                                    }
                                },
                                branch);
                        }
                        else
                        {
                            std::visit(
                                [offset](auto &&b)
                                {
                                    b->update(offset);
                                },
                                branch);
                        }
                    }

                    if (branchHit)
                    {
                        ptr->h += offset;
                        ptr->update();
                        for (auto &&leaf : ptr->leaves)
                        {
                            std::visit(
                                [offset](auto &&l)
                                {
                                    l->update(l->x, l->y + offset);
                                },
                                leaf);
                        }
                    }
                    else if (!leafHit)
                    {
                        for (auto &&leaf : ptr->leaves)
                        {
                            if (std::visit(
                                    [&e, &branchHit](auto &&l)
                                    {
                                        if (events::containsMouse(std::as_const(*l), e))
                                        {
                                            l->eventManager->process(e);
                                            return true;
                                        }
                                        else
                                        {
                                            return false;
                                        }
                                    },
                                    leaf))
                            {
                                break;
                            }
                        }
                    }
                }
            });
        return rptr;
    }

    template <concepts::PositionableType Trunk, concepts::PositionableType... Branches, concepts::PositionableType... Leaves>
    requires concepts::ProcessorType<Trunk> && concepts::TranslatableType<Trunk> &&(concepts::ProcessorType<Branches> &&...) && (concepts::TranslatableType<Branches> && ...) && (concepts::ProcessorType<Leaves> && ...) && (concepts::TranslatableType<Leaves> && ...) //
        void Tree<Trunk, std::tuple<Branches...>, std::tuple<Leaves...>>::update()
    {
        Rectangle::update();
    }

    template <concepts::PositionableType Trunk, concepts::PositionableType... Branches, concepts::PositionableType... Leaves>
    requires concepts::ProcessorType<Trunk> && concepts::TranslatableType<Trunk> &&(concepts::ProcessorType<Branches> &&...) && (concepts::TranslatableType<Branches> && ...) && (concepts::ProcessorType<Leaves> && ...) && (concepts::TranslatableType<Leaves> && ...) //
        void Tree<Trunk, std::tuple<Branches...>, std::tuple<Leaves...>>::update(float dy)
    {
        Rectangle::update(x, y + dy);
        trunk.update(trunk.x, trunk.y + dy);

        for (auto &&branch : branches)
        {
            std::visit(
                [dy](auto &&b)
                {
                    b->update(dy);
                },
                branch);
        }
        for (auto &&leaf : leaves)
        {
            std::visit(
                [dy](auto &&l)
                {
                    l->update(l->x, l->y + dy);
                },
                leaf);
        }
    }

    template <concepts::PositionableType Trunk, concepts::PositionableType... Branches, concepts::PositionableType... Leaves>
    requires concepts::ProcessorType<Trunk> && concepts::TranslatableType<Trunk> &&(concepts::ProcessorType<Branches> &&...) && (concepts::TranslatableType<Branches> && ...) && (concepts::ProcessorType<Leaves> && ...) && (concepts::TranslatableType<Leaves> && ...) //
        void Tree<Trunk, std::tuple<Branches...>, std::tuple<Leaves...>>::update(float dx, float dy)
    {
        Rectangle::update(x + dx, y + dy);

        trunk.update(trunk.x + dx, trunk.y += dy);

        for (auto &&branch : branches)
        {
            std::visit(
                [dy, dx](auto &&b)
                {
                    b->update(b->x + dx, b->y + dy);
                },
                branch);
        }
        for (auto &&leaf : leaves)
        {
            std::visit(
                [dy, dx](auto &&l)
                {
                    l->update(l->x + dx, l->y + dy);
                },
                leaf);
        }
    }

    template <concepts::PositionableType Trunk, concepts::PositionableType... Branches, concepts::PositionableType... Leaves>
    requires concepts::ProcessorType<Trunk> && concepts::TranslatableType<Trunk> &&(concepts::ProcessorType<Branches> &&...) && (concepts::TranslatableType<Branches> && ...) && (concepts::ProcessorType<Leaves> && ...) && (concepts::TranslatableType<Leaves> && ...) //
        void Tree<Trunk, std::tuple<Branches...>, std::tuple<Leaves...>>::draw()
    {
        if (Rectangle::hidden)
            return;

        events::draw(*this);
        trunk.draw();

        if (!open)
            return;

        for (auto &&branch : branches)
        {
            std::visit(
                [](auto &&b)
                {
                    b->draw();
                },
                branch);
        }

        for (auto &&leaf : leaves)
        {
            std::visit(
                [](auto &&l)
                {
                    l->draw();
                },
                leaf);
        }
    }

    template <concepts::PositionableType Trunk, concepts::PositionableType... Branches, concepts::PositionableType... Leaves>
    requires concepts::ProcessorType<Trunk> && concepts::TranslatableType<Trunk> &&(concepts::ProcessorType<Branches> &&...) && (concepts::TranslatableType<Branches> && ...) && (concepts::ProcessorType<Leaves> && ...) && (concepts::TranslatableType<Leaves> && ...) //
        float Tree<Trunk, std::tuple<Branches...>, std::tuple<Leaves...>>::toggle()
    {
        open = !open;

        trunk.nextTexture();

        float dy = -h;

        h = trunk.h;

        if (!open)
        {
            dy += h;
            update();
            return dy;
        }

        for (auto &&branch : branches)
        {
            h += std::visit(
                [](auto &&b)
                {
                    return b->h;
                },
                branch);
        }
        for (auto &&leaf : leaves)
        {
            h += std::visit(
                [](auto &&l)
                {
                    return l->h;
                },
                leaf);
        }
        dy += h;
        update();
        return dy;
    }

    template <concepts::PositionableType Trunk, concepts::PositionableType... Branches, concepts::PositionableType... Leaves>
    requires concepts::ProcessorType<Trunk> && concepts::TranslatableType<Trunk> &&(concepts::ProcessorType<Branches> &&...) && (concepts::TranslatableType<Branches> && ...) && (concepts::ProcessorType<Leaves> && ...) && (concepts::TranslatableType<Leaves> && ...) //
        template <concepts::PositionableType U>
    requires concepts::ProcessorType<U> && concepts::TranslatableType<U>
    void Tree<Trunk, std::tuple<Branches...>, std::tuple<Leaves...>>::addBranch(std::shared_ptr<U> &&u, float offset)
    {
        u->x = x + offset;
        if (branches.empty())
        {
            u->y = trunk.y + trunk.h;
        }
        else
        {
            u->y = std::visit([](auto &&b)
                              { return b->trunk.y + b->trunk.h; },
                              branches.back());
        }

        u->trunk.update(u->x, u->y);

        if (open)
        {
            h += u->h;
            update();
        }
        u->update();

        for (auto &&leaf : leaves)
        {
            std::visit(
                [&u](auto &&l)
                {
                    l->update(l->x, l->y + u->h);
                },
                leaf);
        }

        branches.emplace_back(std::forward<std::shared_ptr<U>>(u));
    }

    template <concepts::PositionableType Trunk, concepts::PositionableType... Branches, concepts::PositionableType... Leaves>
    requires concepts::ProcessorType<Trunk> && concepts::TranslatableType<Trunk> &&(concepts::ProcessorType<Branches> &&...) && (concepts::TranslatableType<Branches> && ...) && (concepts::ProcessorType<Leaves> && ...) && (concepts::TranslatableType<Leaves> && ...) //
        template <concepts::PositionableType U>
    requires concepts::ProcessorType<U> && concepts::TranslatableType<U>
    void Tree<Trunk, std::tuple<Branches...>, std::tuple<Leaves...>>::addLeaf(std::shared_ptr<U> &&u, float offset)
    {

        u->x = x + offset;

        if (branches.empty() && leaves.empty())
        {
            u->y = trunk.y + trunk.h;
        }
        else if (leaves.empty())
        {
            u->y = std::visit([](auto &&b)
                              { return b->trunk.y + b->trunk.h; },
                              branches.back());
        }
        else
        {
            u->y = std::visit([](auto &&l)
                              { return l->y + l->h; },
                              leaves.back());
        }
        if (open)
        {
            h += u->h;
            update();
        }
        u->update();

        leaves.emplace_back(std::forward<std::shared_ptr<U>>(u));
    }

} // namespace gui

#endif