#include "Log2.hh"

namespace opencl
{
std::shared_ptr<gui::Tree> Log2::getOptions()
    {
        std::shared_ptr<gui::Tree> options = gui::Tree::build("OPTIONS");
        return options;
    }
} // namespace opencl
