#include "Slice.hh"

namespace opencl
{
std::shared_ptr<gui::Tree> Slice::getOptions()
    {
        std::shared_ptr<gui::Tree> options = gui::Tree::build("OPTIONS");
        
        slcSliders[0]->resize(0.0f, 0.0f, options->w, 0.0f);
        slcSliders[1]->resize(0.0f, 0.0f, options->w, 0.0f);
        slcSliders[2]->resize(0.0f, 0.0f, options->w, 0.0f);
        
        options->addLeaf(slcSliders[0]);
        options->addLeaf(slcSliders[1]);
        options->addLeaf(slcSliders[2]);

        return options;
    }
} // namespace opencl
