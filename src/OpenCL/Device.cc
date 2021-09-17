#include "Device.hh"

#include <filesystem>
#include <iostream>
#include <string>

#include <gl/glew.h>
#include <gl/gl.h>
#include <gl/glext.h>

#ifdef _WIN32
#include <windows.h>
#endif

#include "Source.hh"
#include "../GUI/Button.hh"

namespace opencl
{

    Device::Device(unsigned int w, unsigned int h) : width(w), height(h)
    {
        // Rounds up to nearest multiple of 32 (for performance concerns)
        width = width + 31 - (width + 31) % 32;
        height = height + 31 - (height + 31) % 32;

        // selectDevice();
    }

    Device::~Device()
    {
    }

    void Device::initialise()
    {
        cQueue = cl::CommandQueue(context, device);

        std::vector<cl::Device> devices{device};
        std::string folder = "./filters/";
        for (const auto &file : std::filesystem::directory_iterator(folder))
        {
            std::string name = file.path().string();
            std::cout << "Found: " << name << std::endl;
            Source src(file.path().string());

            auto f = name.find_last_of('/');
            programs.emplace(name.substr(f + 1, name.find_last_of('.') - f - 1), std::make_shared<Program>(context, src));
            //, "-g -cl-opt-disable -s \"D:\\Documents\\Programming\\Uni\\Thesis\\filters\\raytracing.cl\"");
        }

        std::vector<uint32_t> clr;
        clr.reserve(width * height);
        std::fill_n(std::back_inserter(clr), width * height, 0);

        glGenBuffers(1, &pixelBuffer);
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pixelBuffer);
        glBufferData(GL_PIXEL_UNPACK_BUFFER, width * height * sizeof(GLubyte) * 4, clr.data(), GL_STREAM_DRAW);
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

        try
        {
            outBuffer = (type == CL_DEVICE_TYPE_GPU ? cl::BufferGL(context, CL_MEM_WRITE_ONLY, pixelBuffer) : cl::Buffer(context, CL_MEM_WRITE_ONLY, width * height * sizeof(cl_uint)));
            // outBuffer = cl::Buffer(context, CL_MEM_WRITE_ONLY, width * height * sizeof(cl_uint));
            invMVTransposed = cl::Buffer(context, CL_MEM_READ_ONLY, 12 * sizeof(float));

            programs.at("raytracing")->at("render")->setArg(0, width);
            programs.at("raytracing")->at("render")->setArg(1, height);
            programs.at("raytracing")->at("render")->setArg(2, outBuffer);
            programs.at("raytracing")->at("render")->setArg(7, invMVTransposed);
        }
        catch (const cl::Error &e)
        {
            std::cerr << "Display, " << e.what() << " : " << e.err() << '\n';
            std::terminate();
        }

        GLenum err;
        if ((err = glGetError()))
            std::cout << "Pixel err: " << err << std::endl;
    }

    std::shared_ptr<gui::Tree> Device::buildDeviceTree(float x, float y)
    {
        std::shared_ptr<gui::Tree> dTree = gui::Tree::build("DEVICES");

        std::vector<cl::Platform> platforms;
        cl::Platform::get(&platforms);

        for (const cl::Platform &p : platforms)
        {
            std::shared_ptr<gui::Tree> pTree = gui::Tree::build(p.getInfo<CL_PLATFORM_NAME>().c_str());

            std::vector<cl::Device> devices;
            p.getDevices(CL_DEVICE_TYPE_ALL, &devices);

            for (const cl::Device &d : devices)
            {
                std::shared_ptr<gui::Button> dButton = gui::Button::build(d.getInfo<CL_DEVICE_NAME>().c_str());
                dButton->onPress(
                    [this, p, d]()
                    {
                        platform = p;
                        device = d;
                        selected = true;
                        type = device.getInfo<CL_DEVICE_TYPE>();
                        try
                        {
                            if (type == CL_DEVICE_TYPE_GPU)
                            {
#if defined(__APPLE__)
                                CGLContextObj kCGLContext = CGLGetCurrentContext();
                                CGLShareGroupObj kCGLShareGroup = CGLGetShareGroup(kCGLContext);
                                cl_context_properties props[] =
                                    {
                                        CL_CONTEXT_PROPERTY_USE_CGL_SHAREGROUP_APPLE, (cl_context_properties)kCGLShareGroup,
                                        0};
#else
#ifdef UNIX
                                cl_context_properties props[] =
                                    {
                                        CL_GL_CONTEXT_KHR, (cl_context_properties)glXGetCurrentContext(),
                                        CL_GLX_DISPLAY_KHR, (cl_context_properties)glXGetCurrentDisplay(),
                                        CL_CONTEXT_PLATFORM, (cl_context_properties)platform(),
                                        0};
#else
#ifdef _WIN32

                                cl_context_properties props[] =
                                    {
                                        CL_GL_CONTEXT_KHR, (cl_context_properties)wglGetCurrentContext(),
                                        CL_WGL_HDC_KHR, (cl_context_properties)wglGetCurrentDC(),
                                        CL_CONTEXT_PLATFORM, (cl_context_properties)platform(),
                                        0};
#else
#ifdef __ANDROID__
                                cl_context_properties props[] =
                                    {
                                        CL_GL_CONTEXT_KHR, (cl_context_properties)eglGetCurrentContext(),
                                        CL_EGL_DISPLAY_KHR, (cl_context_properties)eglGetCurrentDisplay(),
                                        CL_CONTEXT_PLATFORM, (cl_context_properties)platform(),
                                        0};
#endif
#endif
#endif
#endif

                                context = cl::Context(device, props);
                            }
                            else
                            {
                                [[maybe_unused]] cl_context_properties props[] =
                                    {
                                        CL_CONTEXT_PLATFORM, (cl_context_properties)platform(),
                                        0};
                                context = cl::Context(device, props);
                            }
                        }
                        catch (const cl::Error &e)
                        {
                            std::cerr << "Build Error, " << e.what() << " : " << e.err() << std::endl;
                            std::terminate();
                        }
                        catch (const std::exception &e)
                        {
                            std::cerr << e.what() << '\n';
                        }
                    });
                pTree->addLeaf(std::move(dButton));
            }

            dTree->addBranch(std::move(pTree));
        }

        dTree->resize(x, y, 0.0f, 0.0f);

        return dTree;
    }

    void Device::selectDevice()
    {
        std::vector<cl::Platform> platforms;
        cl::Platform::get(&platforms);

        std::cout << "Select Platform:\n";

        std::vector<std::string> names;
        std::size_t i = 0;
        for (const cl::Platform &p : platforms)
        {
            std::cout << '[' << i++ << "] " << p.getInfo<CL_PLATFORM_NAME>() << '\n';
        }
        std::cout << std::flush;

        std::size_t n = 0;
        do
        {
            std::cout << "Please enter a number: ";
            while (!(std::cin >> n))
            {
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                std::cout << "Please enter a number: ";
            }
        } while (n >= i);

        platform = platforms.at(n);

        std::vector<cl::Device> devices;
        platform.getDevices(CL_DEVICE_TYPE_ALL, &devices);

        std::cout << "Select Device:\n";

        names.clear();
        i = 0;
        for (const cl::Device &d : devices)
        {
            std::cout << '[' << i++ << "] " << d.getInfo<CL_DEVICE_NAME>() << '\n';
        }
        std::cout << std::flush;

        n = 0;
        do
        {
            std::cout << "Please enter a number: ";
            while (!(std::cin >> n))
            {
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                std::cout << "Please enter a number: ";
            }
        } while (n >= i);

        device = devices.at(n);

        type = device.getInfo<CL_DEVICE_TYPE>();

        try
        {
            if (type == CL_DEVICE_TYPE_GPU)
            {
#if defined(__APPLE__)
                CGLContextObj kCGLContext = CGLGetCurrentContext();
                CGLShareGroupObj kCGLShareGroup = CGLGetShareGroup(kCGLContext);
                cl_context_properties props[] =
                    {
                        CL_CONTEXT_PROPERTY_USE_CGL_SHAREGROUP_APPLE, (cl_context_properties)kCGLShareGroup,
                        0};
#else
#ifdef UNIX
                cl_context_properties props[] =
                    {
                        CL_GL_CONTEXT_KHR, (cl_context_properties)glXGetCurrentContext(),
                        CL_GLX_DISPLAY_KHR, (cl_context_properties)glXGetCurrentDisplay(),
                        CL_CONTEXT_PLATFORM, (cl_context_properties)platform(),
                        0};
#else
#ifdef _WIN32

                cl_context_properties props[] =
                    {
                        CL_GL_CONTEXT_KHR, (cl_context_properties)wglGetCurrentContext(),
                        CL_WGL_HDC_KHR, (cl_context_properties)wglGetCurrentDC(),
                        CL_CONTEXT_PLATFORM, (cl_context_properties)platform(),
                        0};
#else
#ifdef __ANDROID__
                cl_context_properties props[] =
                    {
                        CL_GL_CONTEXT_KHR, (cl_context_properties)eglGetCurrentContext(),
                        CL_EGL_DISPLAY_KHR, (cl_context_properties)eglGetCurrentDisplay(),
                        CL_CONTEXT_PLATFORM, (cl_context_properties)platform(),
                        0};
#endif
#endif
#endif
#endif

                context = cl::Context(device, props);
            }
            else
            {
                [[maybe_unused]] cl_context_properties props[] =
                    {
                        CL_CONTEXT_PLATFORM, (cl_context_properties)platform(),
                        0};
                context = cl::Context(device, props);
            }
        }
        catch (const cl::Error &e)
        {
            std::cerr << "Build Error, " << e.what() << " : " << e.err() << std::endl;
            std::terminate();
        }
        catch (const std::exception &e)
        {
            std::cerr << e.what() << '\n';
        }
    }
} // namespace opencl