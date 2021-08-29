#include "Threshold.hh"

namespace opencl
{
    std::shared_ptr<gui::Tree> Threshold::getOptions()
    {
        std::shared_ptr<gui::Tree> options = gui::Tree::build("OPTIONS");
        thresholdSlider->resize(0.0f, 0.0f, options->w, 0.0f);
        options->addLeaf(thresholdSlider);
        return options;
    }
} // namespace opencl
