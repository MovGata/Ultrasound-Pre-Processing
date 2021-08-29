#include "Shrink.hh"

namespace opencl
{
std::shared_ptr<gui::Tree> Shrink::getOptions()
    {
        std::shared_ptr<gui::Tree> options = gui::Tree::build("OPTIONS");
        return options;
    }
} // namespace opencl
