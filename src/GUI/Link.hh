#ifndef GUI_LINK_HH
#define GUI_LINK_HH

#include <functional>
#include <utility>

#include "../Events/Concepts.hh"

#include "Rectangle.hh"

class Link
{
private:

    gui::Rectangle box;

    std::vector<std::string> inputs;
    std::vector<std::string> outputs;

public:
    Link() = default;
    ~Link() = default;

    void render();

};

#endif