#include "Sqrt.hh"

#include "../Filter.hh"

namespace opencl
{
std::shared_ptr<gui::Tree> Sqrt::getOptions()
    {
        std::shared_ptr<gui::Tree> options = gui::Tree::build("OPTIONS");
        return options;
    }
} // namespace opencl
