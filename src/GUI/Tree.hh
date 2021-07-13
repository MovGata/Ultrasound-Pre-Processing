#ifndef GUI_TREE_HH
#define GUI_TREE_HH

#include <memory>
#include <variant>
#include <vector>

#include <SDL2/SDL_events.h>

#include "MultiTexture.hh"
#include "Rectangle.hh"

#include "../Events/Concepts.hh"
#include "../Events/GUI.hh"
#include "../Events/EventManager.hh"

namespace gui
{

    template <concepts::PositionableType Trunk, typename Branches, typename Leaves>
    class Tree;

    template <concepts::PositionableType Trunk, typename... Branches, typename... Leaves>
    requires concepts::ProcessorType<Trunk> && concepts::TranslatableType<Trunk>
    class Tree<Trunk, std::tuple<Branches...>, std::tuple<Leaves...>> : public Rectangle,
                                                                        public std::enable_shared_from_this<Tree<Trunk, std::tuple<Branches...>, std::tuple<Leaves...>>>
    {
    private:
        std::vector<std::variant<std::shared_ptr<Tree<Branches, Branches..., Leaves...>>...>> branches;
        std::vector<std::variant<std::shared_ptr<Leaves>...>> leaves;

        MultiTexture<Trunk, 2> trunk;

    public:
        float vW = 0, vH = 0;

    protected:
        Tree(Trunk &&t) : trunk(std::forward<Trunk>(t)), vW(t.w), vH(t.h)
        {
            trunk.setTexture(0, std::shared_ptr<Texture>(t.texture));
            trunk.setTexture(1, std::shared_ptr<Texture>(t.texture));

            // em.addCallback(SDL_MOUSEBUTTONDOWN, events::toggle<Trunk>, trunk);

            // em.addCallback(
            // SDL_MOUSEBUTTONDOWN, [](const SDL_Event &e) {}, trunk);

            // em.addCallback(events::GUI_DRAW, MultiTexture<Trunk, 2>::nextTexture, &trunk);
            // em.addCallback(events::GUI_DRAW, events::draw<Trunk>, trunk);
        }

    public:
        events::EventManager eventManager;

        static std::shared_ptr<Tree<Trunk, std::tuple<Branches...>, std::tuple<Leaves...>>> build(Trunk &&t)
        {
            auto rptr = std::shared_ptr<Tree<Trunk, std::tuple<Branches...>, std::tuple<Leaves...>>>(new Tree<Trunk, std::tuple<Branches...>, std::tuple<Leaves...>>(std::forward<Trunk>(t)));
            rptr->eventManager.addCallback(
                events::GUI_REDRAW, [wptr = rptr->weak_from_this()](const SDL_Event &e)
                {
                    auto ptr = wptr.lock();

                    std::pair<float, float> *oldSize = static_cast<std::pair<float, float> *>(e.user.data1);
                    std::pair<float, float> *newSize = static_cast<std::pair<float, float> *>(e.user.data2);

                    auto xd = newSize->first / oldSize->first * (ptr->x + ptr->w) - ptr->w;
                    auto yd = ptr->y * newSize->second / oldSize->second;
                    ptr->h = ptr->h * newSize->second / oldSize->second;
                    ptr->x = xd;
                    ptr->y = yd;
                    ptr->update();

                    ptr->trunk.x = xd;
                    ptr->trunk.y = yd;
                    ptr->trunk.update();
                });
            return rptr;
        }

        Tree() = default;
        ~Tree() = default;

        void draw()
        {
            // trunk.nextTexture();
            if (Rectangle::hidden)
                return;
            events::draw(*this);
            trunk.draw();
        }

        template <concepts::PositionableType U>
        requires concepts::ProcessorType<U> && concepts::TranslatableType<U>
        void addBranch(std::shared_ptr<U> &&u)
        {
            u->x = 0.0f;
            u->y = trunk.y - trunk.h - u->h / 2.0f;
            u->modelview = modelview * glm::translate(glm::mat4(1.0f), {u->x, u->y, 0.0f}) * glm::scale(glm::mat4(1.0f), {u->w, u->h, 0.0f});

            // for (auto &&v : tree->branches)
            // {
            //     std::visit([](auto &&arg)
            //                { u->em.addCallback(SDL_MOUSEBUTTONDOWN, ) });
            // }

            branches.emplace_back(std::forward(u));
        }

        template <concepts::PositionableType U>
        requires concepts::ProcessorType<U> && concepts::TranslatableType<U>
        void addLeaf(std::shared_ptr<U> &&u)
        {
            leaves.emplace_back(std::forward(u));
            float hh, yy;
            if (leaves.empty())
            {
                yy = trunk.y;
                hh = trunk.h;
            }
            else
            {
                yy = std::visit([](auto &&arg)
                                { return arg.y; },
                                leaves.back())
                         .y;
                hh = std::visit([](auto &&arg)
                                { return arg.h; },
                                leaves.back())
                         .h;
            }
            u->x = 0.0f;
            u->y = yy - hh - u->h / 2.0f;
            u->modelview = modelview * glm::translate(glm::mat4(1.0f), {u->x, u->y, 0.0f}) * glm::scale(glm::mat4(1.0f), {u->w, u->h, 0.0f});
            // tree->em.addCallback(events::GUI_DRAW, events::EventManager::process, &u->em);
        }
    };

} // namespace gui

#endif