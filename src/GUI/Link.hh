#ifndef GUI_LINK_HH
#define GUI_LINK_HH

#include <functional>
#include <utility>

#include "Rectangle.hh"

#include "../Events/Concepts.hh"

namespace gui
{

    class Link
    {
    private:
        Rectangle box;

    public:
        glm::vec4 start, end;

        Link() = default;
        ~Link() = default;

        void draw();
    };

} // namespace gui

#endif