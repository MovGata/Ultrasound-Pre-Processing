#ifndef GUI_OPENCL_DEVICE
#define GUI_OPENCL_DEVICE

#include <chrono>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include <gl/glew.h>
#include <gl/gl.h>
#include <gl/glext.h>

#include <CL/cl2.hpp>

#include <SDL2/SDL_timer.h>

#include "Program.hh"
#include "Concepts.hh"

#include "../GUI/Renderer.hh"
#include "../GUI/Rectangle.hh"
#include "../GUI/Tree.hh"

namespace opencl
{

    class Device
    {
    private:
        cl::Platform platform;
        cl::Device device;
        cl::Buffer outBuffer;
        cl::Buffer invMVTransposed;

        unsigned int width = 0;
        unsigned int height = 0;

    public:
        cl_GLuint pixelBuffer;
        cl::CommandQueue cQueue;

        std::map<std::string, std::shared_ptr<Program>> programs;
        cl::Context context;
        cl_device_type type = CL_DEVICE_TYPE_CPU;

        bool selected = false;

        Device(unsigned int width = 512, unsigned int height = 512);
        ~Device();

        void initialise();
        void render(gui::Renderer &renderer);
        void createDisplay(unsigned int w, unsigned int h);
        std::shared_ptr<gui::Tree> buildDeviceTree(float x = 0.0f, float y = 0.0f);
        void selectDevice();
    };

} // namespace opencl
#endif