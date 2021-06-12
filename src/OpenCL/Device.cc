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

Device::Device()
{

    selectDevice();

    cQueue = cl::CommandQueue(context, device);

    std::vector<cl::Device> devices{device};
    std::string folder = "./filters/";
    for (const auto &file : std::filesystem::directory_iterator(folder))
    {
        std::string name = file.path().string();
        std::cout << "Found: " << name << std::endl;
        Source src(file.path().string());

        auto f = name.find_last_of('/');
        programs.emplace(name.substr(f + 1, name.find_last_of('.') - f - 1), std::move(Program(context, src)));
        //, "-g -cl-opt-disable -s \"D:\\Documents\\Programming\\Uni\\Thesis\\filters\\raytracing.cl\"");
    }

    // try
    // {

    // programs.at("raytracing").at("render") = cl::Kernel(program, "render");
    // programs.at("raytracing").at("render").getInfo<CL_KERNEL_FUNCTION_NAME>();
    // }
    // catch (const cl::BuildError &e)
    // {
    //     for (auto &p : e.getBuildLog())
    //     {
    //         std::cout << p.second << std::endl;
    //     }
    //     std::terminate();
    // }
    // catch (const cl::Error &e)
    // {
    //     std::cerr << "we, " << e.what() << " : " << e.err() << '\n';
    // }
    // catch (const std::exception &e)
    // {
    //     std::cerr << "we, " << e.what() << '\n';
    // }

    // try
    // {

    invMVTransposed = cl::Buffer(context, CL_MEM_READ_ONLY, 12 * sizeof(float));
    programs.at("raytracing").at("render").setArg(8, invMVTransposed);
    // }
    // catch (const cl::Error &e)
    // {
    //     std::cerr << "invMatrix, " << e.what() << " : " << e.err() << '\n';
    //     std::terminate();
    // }
}

Device::~Device()
{
}

void Device::render(float *inv, cl_GLuint glPixelBuffer)
{

    // std::cout << "Max work size: " << device.getInfo<CL_DEVICE_MAX_WORK_GROUP_SIZE>() << std::endl;

    cl::NDRange global(512, 512);
    cl::NDRange local(16, 16);

    try
    {
        cl_int err = 0;
        if (type == CL_DEVICE_TYPE_GPU)
        {
            // Share GL buffer.
            glFlush();
            std::vector<cl::Memory> memories;
            memories.push_back(outBuffer);
            err |= cQueue.enqueueAcquireGLObjects(&memories);
            err |= cQueue.enqueueWriteBuffer(invMVTransposed, CL_FALSE, 0, 12 * sizeof(float), inv);
            err |= cQueue.enqueueNDRangeKernel(programs.at("raytracing").at("render"), cl::NullRange, global, local);
            err |= cQueue.enqueueReleaseGLObjects(&memories);
        }
        else
        {
            // Copy via host.
            err |= cQueue.enqueueWriteBuffer(invMVTransposed, CL_FALSE, 0, 12 * sizeof(float), inv);
            err |= cQueue.enqueueNDRangeKernel(programs.at("raytracing").at("render"), cl::NullRange, global, local);
            glBindBuffer(GL_PIXEL_UNPACK_BUFFER, glPixelBuffer);
            GLubyte *p = static_cast<GLubyte *>(glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY));
            auto bSize = outBuffer.getInfo<CL_MEM_SIZE>();
            err |= cQueue.enqueueReadBuffer(outBuffer, CL_TRUE, 0, bSize, p);
            glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
        }

        if (err != CL_SUCCESS)
        {
            std::cerr << err << '\n';
            std::terminate();
        }

        err = cQueue.finish();
        if (err != CL_SUCCESS)
        {
            std::cerr << "clFinish Error: " << err << '\n';
            std::terminate();
        }
    }
    catch (const cl::Error &e)
    {
        std::cerr << "Finish, " << e.what() << " : " << e.err() << '\n';
        std::terminate();
    }
}

void Device::createDisplay(unsigned int w, unsigned int h, cl_GLuint glPixelBuffer)
{

    try
    {
        outBuffer = (type == CL_DEVICE_TYPE_GPU ? cl::BufferGL(context, CL_MEM_WRITE_ONLY, glPixelBuffer) : cl::Buffer(context, CL_MEM_WRITE_ONLY, w * h * sizeof(cl_uint)));
        programs.at("raytracing").at("render").setArg(0, w);
        programs.at("raytracing").at("render").setArg(1, h);
        programs.at("raytracing").at("render").setArg(2, outBuffer);
    }
    catch (const cl::Error &e)
    {
        std::cerr << "Display, " << e.what() << " : " << e.err() << '\n';
        std::terminate();
    }
}

void Device::prepareVolume(unsigned int d, unsigned int l, unsigned int w, float angle, const cl::Buffer &v)
{
    if (v.getInfo<CL_MEM_SIZE>() != l * d * w * 4)
    {
        std::cerr << "invalid volume size." << '\n';
        std::terminate();
    }
    try
    {
        std::cout << "Mem Size: " << v.getInfo<CL_MEM_SIZE>() << std::endl;
        programs.at("raytracing").at("render").setArg(3, d);
        programs.at("raytracing").at("render").setArg(4, l);
        programs.at("raytracing").at("render").setArg(5, w);
        programs.at("raytracing").at("render").setArg(6, angle);
        programs.at("raytracing").at("render").setArg(7, v);
    }
    catch (const cl::Error &e)
    {
        std::cerr << "Volume, " << e.what() << " : " << e.err() << '\n';
        std::terminate();
    }
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
        std::cerr << "Build Error, " << e.what() << " : " << e.err() << '\n';
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }
}