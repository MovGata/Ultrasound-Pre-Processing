#include "Fade.hh"

namespace opencl
{
    std::shared_ptr<gui::Tree> Fade::getOptions()
    {
        std::shared_ptr<gui::Tree> options = gui::Tree::build("OPTIONS");
        // options->addLeaf(Slider::build(0.0f, 0.0f, options->w, 10.0f));
        return options;
    }
} // namespace opencl
