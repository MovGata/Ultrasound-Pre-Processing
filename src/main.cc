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
#include "OpenCL/Kernels/Threshold.hh"

#include "IO/InfoStore.hh"
#include "IO/Types/Binary.hh"
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
            mainWindow.update();
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

    std::shared_ptr<ultrasound::Mindray> reader = std::make_shared<ultrasound::Mindray>();
    if (!reader->load("tests/data/5"))
    {
        std::terminate();
    }

    reader->load();

    reader->volume->sendToCl(device.context, 0);

    std::shared_ptr<gui::Texture> t;

    // DROP ZONE
    auto dropzone = gui::Dropzone::build(wWidth, wHeight);
    mainWindow.addDrawable(std::shared_ptr(dropzone));

    // KERNELS

    int w = 1, h = 1;
    TTF_SizeText(font, "> KERNELS", &w, &h);
    t = std::make_shared<gui::Texture>(w + 2, h + 2);
    t->addText(font, "> KERNELS");

    auto alt = std::make_shared<gui::Texture>(*t);
    auto rec = std::make_shared<gui::Rectangle>(wWidth - 2.0f * (static_cast<float>(w) + 2.0f), 0.0f, static_cast<float>(w) + 2.0f, static_cast<float>(h) + 2.0f, std::move(t));

    int oneWidth;
    TTF_SizeText(font, ">", &oneWidth, nullptr);

    alt->rotate(1, 1, oneWidth, oneWidth);

    auto tree = gui::Tree::build(std::move(rec), std::move(alt));
    tree->Rectangle::update(wWidth - 2.0f * (static_cast<float>(w) + 2.0f), 0.0f, tree->w, 0.0f);
    tree->texture->fill({0x2C, 0x2C, 0x2C, 0xFF});

    auto inputTree = gui::Tree::build("INPUTS");
    auto outputTree = gui::Tree::build("OUTPUTS");
    auto dataTree = gui::Tree::build("DATA");

    tree->addBranch(std::shared_ptr(inputTree), 4.0f);
    tree->addBranch(std::shared_ptr(outputTree), 4.0f);
    tree->addBranch(std::shared_ptr(dataTree), 4.0f);

    auto polar = std::make_shared<opencl::ToPolar>(device.context, device.cQueue, device.programs.at("cartesian")->at("toSpherical"));
    auto cartesian = std::make_shared<opencl::ToCartesian>(device.context, device.cQueue, device.programs.at("cartesian")->at("toCartesian"));
    auto slice = std::make_shared<opencl::Slice>(device.context, device.cQueue, device.programs.at("utility")->at("slice"));
    auto threshold = std::make_shared<opencl::Threshold>(device.context, device.cQueue, device.programs.at("utility")->at("threshold"));
    auto invert = std::make_shared<opencl::Invert>(device.context, device.cQueue, device.programs.at("utility")->at("invert"));
    auto contrast = std::make_shared<opencl::Contrast>(device.context, device.cQueue, device.programs.at("utility")->at("contrast"));
    auto log = std::make_shared<opencl::Log2>(device.context, device.cQueue, device.programs.at("utility")->at("logTwo"));
    auto shrink = std::make_shared<opencl::Shrink>(device.context, device.cQueue, device.programs.at("utility")->at("shrink"));
    auto fade = std::make_shared<opencl::Fade>(device.context, device.cQueue, device.programs.at("utility")->at("fade"));
    auto sqrt = std::make_shared<opencl::Sqrt>(device.context, device.cQueue, device.programs.at("utility")->at("square"));
    auto clamp = std::make_shared<opencl::Clamp>(device.context, device.cQueue, device.programs.at("utility")->at("clamping"));

    auto binary = std::make_shared<io::Binary>(device.cQueue);

    auto mindray = gui::Kernel::buildButton<std::shared_ptr<gui::Dropzone>>("MINDRAY", mainWindow.kernel, mainWindow.renderers, dropzone, std::move(reader));

    auto toPolar = gui::Kernel::buildButton<std::shared_ptr<gui::Dropzone>>("To Polar", mainWindow.kernel, mainWindow.renderers, dropzone, polar);
    auto toCartesian = gui::Kernel::buildButton<std::shared_ptr<gui::Dropzone>>("To Cartesian", mainWindow.kernel, mainWindow.renderers, dropzone, cartesian);
    auto sliceK = gui::Kernel::buildButton<std::shared_ptr<gui::Dropzone>>("Slice", mainWindow.kernel, mainWindow.renderers, dropzone, slice);
    auto threshK = gui::Kernel::buildButton<std::shared_ptr<gui::Dropzone>>("Threshold", mainWindow.kernel, mainWindow.renderers, dropzone, threshold);
    auto invK = gui::Kernel::buildButton<std::shared_ptr<gui::Dropzone>>("Invert", mainWindow.kernel, mainWindow.renderers, dropzone, invert);
    auto clampK = gui::Kernel::buildButton<std::shared_ptr<gui::Dropzone>>("Clamp", mainWindow.kernel, mainWindow.renderers, dropzone, clamp);
    auto conK = gui::Kernel::buildButton<std::shared_ptr<gui::Dropzone>>("Contrast", mainWindow.kernel, mainWindow.renderers, dropzone, contrast);
    auto logK = gui::Kernel::buildButton<std::shared_ptr<gui::Dropzone>>("Log2", mainWindow.kernel, mainWindow.renderers, dropzone, log);
    auto shrinkK = gui::Kernel::buildButton<std::shared_ptr<gui::Dropzone>>("Shrink", mainWindow.kernel, mainWindow.renderers, dropzone, shrink);
    auto fadeK = gui::Kernel::buildButton<std::shared_ptr<gui::Dropzone>>("Fade", mainWindow.kernel, mainWindow.renderers, dropzone, fade);
    auto sqrtK = gui::Kernel::buildButton<std::shared_ptr<gui::Dropzone>>("Sqrt", mainWindow.kernel, mainWindow.renderers, dropzone, sqrt);

    auto outputButton = gui::Kernel::buildButton<std::shared_ptr<gui::Dropzone>>("Binary", mainWindow.kernel, mainWindow.renderers, dropzone, binary);

    inputTree->addLeaf(std::move(mindray), 4.0f);
    dataTree->addLeaf(std::move(toPolar), 4.0f);
    dataTree->addLeaf(std::move(toCartesian), 4.0f);
    dataTree->addLeaf(std::move(sliceK), 4.0f);
    dataTree->addLeaf(std::move(threshK), 4.0f);
    dataTree->addLeaf(std::move(invK), 4.0f);
    dataTree->addLeaf(std::move(clampK), 4.0f);
    dataTree->addLeaf(std::move(conK), 4.0f);
    dataTree->addLeaf(std::move(logK), 4.0f);
    dataTree->addLeaf(std::move(shrinkK), 4.0f);
    dataTree->addLeaf(std::move(fadeK), 4.0f);
    dataTree->addLeaf(std::move(sqrtK), 4.0f);
    outputTree->addLeaf(std::move(outputButton), 4.0f);

    mainWindow.addDrawable(std::shared_ptr(tree));

    // HIDE BUTTON

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
    eventCount = 1;

    while (!quit)
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

        if (eventCount) // If no events have occurred, no changes to drawing have occurred.
        {
            // mainWindow.setActive();

            // Share GL buffers.
            glFlush();

            // Run OpenCL stuff
            for (auto &&renderer : mainWindow.renderers)
            {
                if (renderer->modified)
                {
                    renderer->updateView();

                    auto start = std::chrono::steady_clock::now();
                    device.render(*renderer);
                    auto stop = std::chrono::steady_clock::now();
                    std::cout << "Kernel Execution Time: " << std::chrono::duration_cast<std::chrono::duration<float, std::milli>>(stop - start).count() << "ms" << std::endl;

                    renderer->texture->update(device.pixelBuffer);
                }
            }

            // Set renders to finished processing
            for (auto &&renderer : mainWindow.renderers)
            {
                renderer->modified = false;
            }

            // Run OpenGL stuff
            mainWindow.update();
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

    return EXIT_SUCCESS;
}
