#include "ToPolar.hh"

namespace opencl
{
std::shared_ptr<gui::Tree> ToPolar::getOptions()
    {
        std::shared_ptr<gui::Tree> options = gui::Tree::build("OPTIONS");
        return options;
    }
} // namespace opencl
