#ifndef GUI_TREE_HH
#define GUI_TREE_HH

#include <memory>
#include <variant>
#include <vector>

#include <SDL2/SDL_events.h>

#include "MultiTexture.hh"
#include "Rectangle.hh"

namespace gui
{
    class Tree : public Rectangle, public std::enable_shared_from_this<Tree>
    {
    public:
        using branch_t = std::shared_ptr<Tree>;
        using leaf_t = std::shared_ptr<Rectangle>;
    private:
        std::vector<branch_t> branches;
        std::vector<leaf_t> leaves;

        MultiTexture<Rectangle, 2> trunk;

        bool open = false;

    protected:
        Tree(leaf_t &&t);
        Tree(leaf_t &&t, std::shared_ptr<Texture> &&alt);

    public:
        std::weak_ptr<events::EventManager> subManager;

        static std::shared_ptr<Tree> build(const std::string &str);
        static std::shared_ptr<Tree> build(leaf_t &&t);
        static std::shared_ptr<Tree> build(leaf_t &&t, std::shared_ptr<Texture> &&alt);

        Tree() = default;
        ~Tree() = default;

        void draw();

        float toggle();
        void update(float dx = 0.0f, float dy = 0.0f, float dw = 0.0f, float dh = 0.0f);

        void addBranch(branch_t &&, float offset = 0.0f);
        void addLeaf(leaf_t &&, float offset = 0.0f);
    };

} // namespace gui

#endif