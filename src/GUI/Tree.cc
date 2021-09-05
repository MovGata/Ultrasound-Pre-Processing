#include "Tree.hh"

#include "../Events/GUI.hh"

namespace gui
{
    Tree::Tree(leaf_t &&t) : Tree(std::forward<leaf_t>(t), std::shared_ptr<Texture>(t->texture))
    {
    }

    Tree::Tree(leaf_t &&t, std::shared_ptr<Texture> &&alt) : trunk(*t)
    {
        trunk.setTexture(0, std::shared_ptr<Texture>(t->texture));
        trunk.setTexture(1, std::forward<std::shared_ptr<Texture>>(alt));

        w = trunk.w;
        h = trunk.h;
        Rectangle::update();
    }

    std::shared_ptr<Tree> Tree::build(const std::string &str)
    {
        int w, h;
        TTF_SizeText(Texture::lastFont, str.c_str(), &w, &h);
        auto t = std::make_shared<gui::Texture>(w + 2, h + 2);
        t->addText(Texture::lastFont, str.c_str());

        auto rec = std::make_shared<gui::Rectangle>(0.0f, 0.0f, static_cast<float>(w) + 2.0f, static_cast<float>(h) + 2.0f, std::move(t));
        rec->update();

        return build(std::move(rec));
    }

    std::shared_ptr<Tree> Tree::build(leaf_t &&t)
    {
        return build(std::forward<leaf_t>(t), std::shared_ptr<Texture>(t->texture));
    }

    std::shared_ptr<Tree> Tree::build(leaf_t &&t, std::shared_ptr<Texture> &&alt)
    {
        auto rptr = std::shared_ptr<Tree>(new Tree(std::forward<leaf_t>(t), std::forward<std::shared_ptr<Texture>>(alt)));
        rptr->Rectangle::draw = std::bind(Tree::draw, rptr.get());
        rptr->Rectangle::resize = std::bind(Tree::update, rptr.get(), std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);

        rptr->eventManager->addCallback(
            events::GUI_REDRAW, [wptr = rptr->weak_from_this()](const SDL_Event &e)
            {
                auto ptr = wptr.lock();

                std::pair<float, float> *oldSize = static_cast<std::pair<float, float> *>(e.user.data1);
                std::pair<float, float> *newSize = static_cast<std::pair<float, float> *>(e.user.data2);

                float xd = newSize->first / oldSize->first * (ptr->x + ptr->w) - ptr->w;
                float yd = ptr->y * newSize->second / oldSize->second;
                float offsetY = yd - ptr->y;
                float offsetX = xd - ptr->x;

                ptr->resize(offsetX, offsetY, 0.0f, 0.0f);

                // for (auto &&branch : ptr->branches)
                //     branch->resize(offsetX, offsetY, 0.0f, 0.0f);

                // for (auto &&leaf : ptr->leaves)
                //     leaf->resize(offsetX, offsetY, 0.0f, 0.0f);

                // ptr->Rectangle::update(xd, yd);
                // ptr->trunk.update(xd, yd);
            });
        rptr->eventManager->addCallback(
            SDL_MOUSEBUTTONDOWN, [wptr = rptr->weak_from_this()](const SDL_Event &e)
            {
                auto ptr = wptr.lock();

                float offset = 0;
                if (events::containsMouse(ptr->trunk, e))
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
                            if (events::containsMouse(branch->trunk, e))    // Hit branch header
                            {
                                offset = -branch->h;                        // Subtract size before closing/opening
                                branch->eventManager->process(e);
                                ptr->subManager = branch->eventManager;
                                offset += branch->h;                        // Add size after opening/closing
                                branchHit = true;
                            }
                            else if (events::containsMouse(*branch, e))     // Hit branch sub-item
                            {
                                branch->eventManager->process(e);
                                ptr->subManager = branch->eventManager;
                                leafHit = true;
                            }
                        }
                        else
                        {
                            branch->resize(0.0f, offset, 0.0f, 0.0f);
                        }
                    }

                    if (branchHit)
                    {
                        ptr->Rectangle::update(0.0f, 0.0f, 0.0f, offset);
                        for (auto &&leaf : ptr->leaves)
                        {
                            leaf->resize(0.0f, offset, 0.0f, 0.0f);
                        }
                    }
                    else if (!leafHit)
                    {
                        for (auto &&leaf : ptr->leaves)
                        {
                            if (events::containsMouse(*leaf, e))
                            {
                                leaf->eventManager->process(e);
                                ptr->subManager = leaf->eventManager;
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

    void Tree::update(float dx, float dy, float dw, float dh)
    {
        Rectangle::update(dx, dy, dw, dh);
        trunk.update(dx, dy, dw, dh);

        for (auto &&branch : branches)
        {
            branch->resize(dx, dy, dw, dh);
        }
        for (auto &&leaf : leaves)
        {
            leaf->resize(dx, dy, dw, dh);
        }
    }

    void Tree::draw()
    {
        if (Rectangle::hidden)
            return;

        Rectangle::upload();
        trunk.upload();

        if (!open)
            return;

        for (auto &&branch : branches)
            branch->draw();

        for (auto &&leaf : leaves)
            leaf->draw();
    }

    float Tree::toggle()
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
            h += branch->h;

        for (auto &&leaf : leaves)
            h += leaf->h;
        
        dy += h;
        update();
        return dy;
    }

    void Tree::addBranch(branch_t &&u, float offset)
    {
        float xOff = x + offset;
        float yOff = 0.0f;
        // u->x = x + offset;
        if (branches.empty())
        {
            yOff = trunk.y + trunk.h;
            // u->y = trunk.y + trunk.h;
        }
        else
        {
            branch_t back = branches.back();
            yOff = back->trunk.y + back->trunk.h;
            // u->y = back->trunk.y + back->trunk.h;
        }

        // u->trunk.update(u->x, u->y);
        u->resize(xOff, yOff, 0.0f, 0.0f);

        if (open)
        {
            // h += u->h;
            update(0.0f, 0.0f, 0.0f, u->h);
        }
        // u->update();

        for (auto &&leaf : leaves)
        {
            leaf->resize(0.0f, u->h, 0.0f, 0.0f);
        }

        branches.emplace_back(std::forward<branch_t>(u));
    }

    void Tree::addLeaf(leaf_t &&u, float offset)
    {
        float yOff = 0.0f;
        if (branches.empty() && leaves.empty())
        {
            yOff = trunk.y + trunk.h;
        }
        else if (leaves.empty())
        {
            auto back = branches.back();
            yOff = back->trunk.y + back->trunk.h;
        }
        else
        {
            auto back = leaves.back();
            yOff = back->y + back->h;
        }

        if (open)
        {
            h += u->h;
            update();
        }

        u->resize(x + offset - u->x, yOff - u->y, 0.0f, 0.0f);

        leaves.emplace_back(std::forward<leaf_t>(u));
    }
} // namespace gui
