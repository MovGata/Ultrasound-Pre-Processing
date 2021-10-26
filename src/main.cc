#include <chrono>
#include <cstddef>
#include <iostream>
#include <thread>
#include <chrono>

#include "GUI/Instance.hh"
#include "GUI/Tree.hh"
#include "GUI/Rectangle.hh"
#include "GUI/Window.hh"
#include "GUI/Button.hh"
#include "GUI/Dropzone.hh"
#include "GUI/Renderer.hh"

#include "OpenCL/Device.hh"
#include "OpenCL/Kernel.hh"

#include "OpenCL/Kernels/ToPolar.hh"
#include "OpenCL/Kernels/ToCartesian.hh"
#include "OpenCL/Kernels/Slice.hh"
#include "OpenCL/Kernels/Invert.hh"
#include "OpenCL/Kernels/Contrast.hh"
#include "OpenCL/Kernels/Log2.hh"
#include "OpenCL/Kernels/Shrink.hh"
#include "OpenCL/Kernels/Fade.hh"
#include "OpenCL/Kernels/Sqrt.hh"
#include "OpenCL/Kernels/Clamp.hh"
#include "OpenCL/Kernels/Colourise.hh"
#include "OpenCL/Kernels/Threshold.hh"
#include "OpenCL/Kernels/MedianNoise.hh"
#include "OpenCL/Kernels/Gaussian.hh"

#include "IO/InfoStore.hh"
#include "IO/Types/Binary.hh"
#include "IO/Types/Nifti1.hh"
#include "Ultrasound/Mindray.hh"

#include "Data/Volume.hh"

#include "glm/ext.hpp"

int main([[maybe_unused]] int argc, [[maybe_unused]] char **argv)
{

    std::ios::sync_with_stdio(false);

    using gui::Window;

    gui::Instance init;
    gui::Window mainWindow(1024, 768);
    init.initGL();

    mainWindow.redraw();

    TTF_Font *font = init.loadFont("./res/fonts/cour.ttf");
    gui::Texture::lastFont = font;

    auto size = mainWindow.getSize();
    float wWidth = static_cast<float>(size.first);
    float wHeight = static_cast<float>(size.second);

    opencl::Device device;

    {
        int w, h;
        TTF_SizeText(gui::Texture::lastFont, "Select a Device:", &w, &h);
        std::shared_ptr<gui::Texture> dTexture = std::make_shared<gui::Texture>(w, h);
        dTexture->addText("Select a Device:");

        std::shared_ptr<gui::Rectangle> dTitle = std::make_shared<gui::Rectangle>(0.0f, 0.0f, dTexture->textureW, dTexture->textureH, std::move(dTexture));
        dTitle->update();
        mainWindow.addDrawable(std::move(dTitle));
        mainWindow.addDrawable(device.buildDeviceTree(0.0f, static_cast<float>(h) + 10.0f));
    }

    auto timeA = SDL_GetTicks();
    bool quit = false;

    // Device Selection Loop
    std::size_t eventCount = 1;
    while (!quit && !device.selected)
    {
        SDL_Event e;
        while (SDL_PollEvent(&e))
        {
            ++eventCount;
            if (e.type == SDL_QUIT)
            {
                quit = true;
                break;
            }
            else if (e.type == SDL_WINDOWEVENT)
            {
                if (e.window.event == SDL_WINDOWEVENT_CLOSE)
                {
                    quit = true;
                    break;
                }
                else
                {
                    if (e.window.windowID == mainWindow.getID())
                    {
                        mainWindow.process(e);
                    }
                }
            }
            else
            {
                mainWindow.process(e);
            }
        }

        if (eventCount) // If no events have occurred, no changes to drawing have occurred.
        {
            // Share GL buffers.
            glFlush();
            // Run OpenGL stuff
            mainWindow.clean();
            mainWindow.draw();
        }

        eventCount = 0;

        auto duration = long(timeA) + long(1000.0 / 30.0) - long(SDL_GetTicks());
        if (duration - 2 > 0)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(duration - 2)); // Sleep with 2ms left over.
        }
        while (SDL_GetTicks() < timeA + 1000 / 30)
            ; // Busy wait last 2 ms.
        timeA = SDL_GetTicks();
    }

    if (quit)
    {
        return EXIT_SUCCESS;
    }

    device.initialise();

    mainWindow.drawables.clear();

    std::shared_ptr<gui::Texture> t;

    // Drop Zone
    auto dropzone = gui::Dropzone::build(wWidth, wHeight);
    mainWindow.addDrawable(std::shared_ptr(dropzone));

    // Kernel Tree
    int w = 1, h = 1;
    TTF_SizeText(font, "KERNELS", &w, &h);
    t = std::make_shared<gui::Texture>(w + 2, h + 2);
    t->addText(font, "KERNELS");

    auto rec = std::make_shared<gui::Rectangle>(wWidth - 2.0f * (static_cast<float>(w) + 2.0f), 0.0f, static_cast<float>(w) + 2.0f, static_cast<float>(h) + 2.0f, std::move(t));

    int oneWidth, oneHeight;
    TTF_SizeText(font, ">", &oneWidth, &oneHeight);

    auto tree = gui::Tree::build(std::move(rec));
    tree->Rectangle::update(wWidth - 2.0f * (static_cast<float>(w) + 2.0f), 0.0f, tree->w, 0.0f);
    tree->texture->fill({0x2C, 0x2C, 0x2C, 0xFF});

    auto inputTree = gui::Tree::build("INPUTS");
    auto outputTree = gui::Tree::build("OUTPUTS");
    auto dataTree = gui::Tree::build("DATA");

    tree->addBranch(std::shared_ptr(inputTree), 4.0f);
    tree->addBranch(std::shared_ptr(outputTree), 4.0f);
    tree->addBranch(std::shared_ptr(dataTree), 4.0f);


    auto reader = std::make_shared<ultrasound::Mindray>(device.context);
    inputTree->addLeaf(dropzone->buildKernel("MINDRAY", mainWindow.kernel, mainWindow.renderers, std::move(reader)), 4.0f);

    auto polar      = std::make_shared<opencl::ToPolar>(device.context, device.cQueue, device.programs.at("cartesian")->at("toSpherical"));
    auto cartesian  = std::make_shared<opencl::ToCartesian>(device.context, device.cQueue, device.programs.at("cartesian")->at("toCartesian"));
    auto slice      = std::make_shared<opencl::Slice>(device.context, device.cQueue, device.programs.at("utility")->at("slice"));
    auto threshold  = std::make_shared<opencl::Threshold>(device.context, device.cQueue, device.programs.at("utility")->at("threshold"));
    auto invert     = std::make_shared<opencl::Invert>(device.context, device.cQueue, device.programs.at("utility")->at("invert"));
    auto contrast   = std::make_shared<opencl::Contrast>(device.context, device.cQueue, device.programs.at("utility")->at("contrast"));
    auto log        = std::make_shared<opencl::Log2>(device.context, device.cQueue, device.programs.at("utility")->at("logTwo"));
    auto shrink     = std::make_shared<opencl::Shrink>(device.context, device.cQueue, device.programs.at("utility")->at("shrink"));
    auto fade       = std::make_shared<opencl::Fade>(device.context, device.cQueue, device.programs.at("utility")->at("fade"));
    auto sqrt       = std::make_shared<opencl::Sqrt>(device.context, device.cQueue, device.programs.at("utility")->at("square"));
    auto clamp      = std::make_shared<opencl::Clamp>(device.context, device.cQueue, device.programs.at("utility")->at("clamping"));
    auto colourise  = std::make_shared<opencl::Colourise>(device.context, device.cQueue, device.programs.at("utility")->at("colourise"));
    auto median     = std::make_shared<opencl::Median>(device);
    auto gaussian   = std::make_shared<opencl::Gaussian>(device);

    dataTree->addLeaf(dropzone->buildKernel("To Polar", mainWindow.kernel, mainWindow.renderers, polar), 4.0f);
    dataTree->addLeaf(dropzone->buildKernel("To Cartesian", mainWindow.kernel, mainWindow.renderers, cartesian), 4.0f);
    dataTree->addLeaf(dropzone->buildKernel("Slice", mainWindow.kernel, mainWindow.renderers, slice), 4.0f);
    dataTree->addLeaf(dropzone->buildKernel("Threshold", mainWindow.kernel, mainWindow.renderers, threshold), 4.0f);
    dataTree->addLeaf(dropzone->buildKernel("Invert", mainWindow.kernel, mainWindow.renderers, invert), 4.0f);
    dataTree->addLeaf(dropzone->buildKernel("Clamp", mainWindow.kernel, mainWindow.renderers, clamp), 4.0f);
    dataTree->addLeaf(dropzone->buildKernel("Contrast", mainWindow.kernel, mainWindow.renderers, contrast), 4.0f);
    dataTree->addLeaf(dropzone->buildKernel("Log2", mainWindow.kernel, mainWindow.renderers, log), 4.0f);
    dataTree->addLeaf(dropzone->buildKernel("Shrink", mainWindow.kernel, mainWindow.renderers, shrink), 4.0f);
    dataTree->addLeaf(dropzone->buildKernel("Fade", mainWindow.kernel, mainWindow.renderers, fade), 4.0f);
    dataTree->addLeaf(dropzone->buildKernel("Sqrt", mainWindow.kernel, mainWindow.renderers, sqrt), 4.0f);
    dataTree->addLeaf(dropzone->buildKernel("Colourise", mainWindow.kernel, mainWindow.renderers, colourise), 4.0f);
    dataTree->addLeaf(dropzone->buildKernel("Median", mainWindow.kernel, mainWindow.renderers, median), 4.0f);
    dataTree->addLeaf(dropzone->buildKernel("Gaussian", mainWindow.kernel, mainWindow.renderers, gaussian), 4.0f);

    auto binary = std::make_shared<io::Binary>(device.cQueue);
    auto nifti1 = std::make_shared<io::Nifti1>(device.cQueue);

    outputTree->addLeaf(dropzone->buildKernel("Binary", mainWindow.kernel, mainWindow.renderers, binary), 4.0f);
    outputTree->addLeaf(dropzone->buildKernel("Nifti1", mainWindow.kernel, mainWindow.renderers, nifti1), 4.0f);

    mainWindow.addDrawable(std::shared_ptr(tree));

    // Hide Button
    t = std::make_shared<gui::Texture>(40, 40);
    t->fill({0x4E, 0x4E, 0x4E, 0xFF});
    std::shared_ptr<gui::Button> hideButton = gui::Button::build({wWidth - 40.0f, wHeight - 20.0f, 40.0f, 20.0f, std::move(t)});
    hideButton->onPress(
        [wptr = dropzone->weak_from_this(), wptr2 = tree->weak_from_this()]()
        {
            auto ptr = wptr.lock();
            auto ptr2 = wptr2.lock();
            ptr->hidden = !ptr->hidden;
            ptr2->hidden = !ptr2->hidden;
        });

    mainWindow.addDrawable(std::shared_ptr(hideButton));

    timeA = SDL_GetTicks();

    //Main Loop
    while (!quit)
    {

        // Process events first

        SDL_Event e;
        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT)
            {
                quit = true;
                break;
            }
            else if (e.type == SDL_WINDOWEVENT) // Fullscreen, minimise, resize, etc...
            {
                if (e.window.event == SDL_WINDOWEVENT_CLOSE)
                {
                    quit = true;
                    break;
                }
                else
                {
                    if (e.window.windowID == mainWindow.getID())
                    {
                        mainWindow.process(e);
                    }
                }
            }
            else
            {
                if (mainWindow.kernel != nullptr)
                {
                    mainWindow.kernel->eventManager->process(e);
                    if (e.type == SDL_MOUSEBUTTONUP)
                    {
                        mainWindow.kernel.reset<gui::Kernel>(nullptr);
                    }
                }
                else
                {
                    mainWindow.process(e);
                }
            }
        }

        // Prepare GL buffers for interop.
        glFlush();

        // If renderers are at the same frame, we can save disk loading costs.
        auto sortedRenderers = mainWindow.renderers;
        std::sort(sortedRenderers.begin(), sortedRenderers.end(), [](const std::shared_ptr<gui::Renderer> &sa, const std::shared_ptr<gui::Renderer> &sb)
                  { return sa->cFrame < sb->cFrame; });

        int lastR = -1;
        for (auto &&renderer : sortedRenderers)
        {
            if (renderer->modified)
            {
                if (lastR == -1 || static_cast<cl_uint>(lastR) != renderer->rFrame)
                {
                    gui::Kernel::executeKernels(renderer->rFrame); // Run kernels at specified frame
                    lastR = renderer->rFrame;
                }

                renderer->updateView(); // Update rotation, translation, scale

                auto start = std::chrono::steady_clock::now();
                device.render(*renderer); // 3D -> 2D render
                auto stop = std::chrono::steady_clock::now();
                std::cout << "Kernel Execution Time: " << std::chrono::duration_cast<std::chrono::duration<float, std::milli>>(stop - start).count() << "ms" << std::endl;

                renderer->addFrame(device.pixelBuffer); // Save 2D frame into rendered footage
            }
        }

        // Run OpenGL stuff
        mainWindow.clean();
        mainWindow.draw();

        // Sleep to minimise CPU cycles
        auto duration = long(timeA) + long(1000.0 / 30.0) - long(SDL_GetTicks());
        if (duration - 2 > 0)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(duration - 2)); // Sleep with 2ms left over.
        }
        while (SDL_GetTicks() < timeA + 1000 / 30)
            ; // Busy wait last 2 ms to not oversleep.
        timeA = SDL_GetTicks();
    }

    return EXIT_SUCCESS;
}
