#ifndef GUI_KERNEL_HH
#define GUI_KERNEL_HH

#include <functional>
#include <string>
#include <memory>
#include <vector>

#include <glm/ext.hpp>

#include "Rectangle.hh"
#include "Button.hh"
#include "Texture.hh"
// #include "Renderer.hh"
#include "Tree.hh"
#include "Slider.hh"

#include "../Data/Volume.hh"
#include "../OpenCL/Kernel.hh"
#include "../OpenCL/Filter.hh"
#include "../events/EventManager.hh"
#include "../OpenCL/Concepts.hh"

#include "../OpenCL/Kernels/ToPolar.hh"
#include "../OpenCL/Kernels/ToCartesian.hh"
#include "../OpenCL/Kernels/Slice.hh"
#include "../OpenCL/Kernels/Invert.hh"
#include "../OpenCL/Kernels/Contrast.hh"
#include "../OpenCL/Kernels/Log2.hh"
#include "../OpenCL/Kernels/Shrink.hh"
#include "../OpenCL/Kernels/Fade.hh"
#include "../OpenCL/Kernels/Sqrt.hh"
#include "../OpenCL/Kernels/Clamp.hh"
#include "../OpenCL/Kernels/Threshold.hh"

#include "../Ultrasound/Mindray.hh"

namespace
{
    using namespace opencl;
    using namespace ultrasound;
}

namespace gui
{

    class Renderer;
    class Dropzone;

    class Kernel : public Rectangle, public std::enable_shared_from_this<Kernel>
    {
    private:
        Kernel(std::shared_ptr<opencl::Filter> &&f, std::shared_ptr<Texture> &&tptr);

        bool move = true;
        std::weak_ptr<events::EventManager> optionEvent;
        bool active = false;
        bool modified = true;
        std::shared_ptr<data::Volume> volume = std::make_shared<data::Volume>();

    public:
        std::shared_ptr<opencl::Filter> filter;
        std::function<void(std::shared_ptr<data::Volume> &, bool)> fire;
        std::function<void(std::shared_ptr<data::Volume> &)> arm;
        static std::vector<std::weak_ptr<Kernel>> xKernels;

        std::shared_ptr<Button> inNode;
        std::shared_ptr<Button> outNode;
        std::shared_ptr<Button> renderButton;

        gui::Rectangle outLine;
        gui::Rectangle title;

        std::shared_ptr<Kernel> inLink;
        std::shared_ptr<Kernel> outLink;

        std::shared_ptr<Tree> options;

        bool link = false;


        void updateLine(float ox, float oy);
        void draw();

        static std::shared_ptr<Kernel> build(std::shared_ptr<opencl::Filter> &&f, std::shared_ptr<Texture> &&tptr);        

        ~Kernel() = default;

        void execute(std::shared_ptr<data::Volume> &sp, bool m);
        void update(float, float, float, float);

        bool endLink(const SDL_Event &e, std::shared_ptr<Kernel> &k);
        std::shared_ptr<Renderer> buildRenderer(std::vector<std::shared_ptr<Renderer>> &wr);

        static void executeKernels(cl_uint i);
    };

}
#endif