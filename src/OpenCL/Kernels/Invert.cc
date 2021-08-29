#include "Invert.hh"

namespace opencl
{
std::shared_ptr<gui::Tree> Invert::getOptions()
    {
        std::shared_ptr<gui::Tree> options = gui::Tree::build("OPTIONS");
        return options;
    }
} // namespace opencl
