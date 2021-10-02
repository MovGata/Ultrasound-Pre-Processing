#ifndef GUI_RENDERER_HH
#define GUI_RENDERER_HH

#include <functional>
#include <memory>

#include <GL/glew.h>
#include <GL/gl.h>

#include "Button.hh"
#include "Slider.hh"
#include "Rectangle.hh"
#include "Kernel.hh"

#include "../Data/Volume.hh"

#include "../Events/Concepts.hh"
#include "../Events/EventManager.hh"

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

namespace gui
{
    class Renderer : public Rectangle, public std::enable_shared_from_this<Renderer>
    {
    private:
        std::shared_ptr<Button> closeButton;
        std::shared_ptr<Button> pauseButton;
        std::shared_ptr<Slider> progressBar;
        Uint32 lastTick = 0;

        Renderer(Rectangle &&d, std::shared_ptr<data::Volume> &&ptr, std::shared_ptr<Kernel> &&krnl);

    public:
        ~Renderer() = default;

        std::shared_ptr<data::Volume> tf;
        std::vector<std::vector<cl_uchar4>> video;
        glm::mat4 lastview;
        glm::vec3 translation = {0.0f, 0.0f, 5.0f};

        bool modified = false;
        bool paused = false;

        std::array<float, 12> inv = {0};

        std::shared_ptr<Kernel> kernel;
        cl_uint cFrame = 0;
        cl_uint rFrame = 0;

        static std::shared_ptr<Renderer> build(std::vector<std::shared_ptr<Renderer>> &wr, Rectangle &&d, std::shared_ptr<data::Volume> &&ptr, std::shared_ptr<Kernel> &&krnl);
        
        void updateView();
        void update(float xx = 0.0f, float yy = 0.0f, float ww = 0.0f, float hh = 0.0f);
        void addFrame(GLuint pixelBuffer);
        void draw();
    };

}

#endif