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
#include "Ultrasound/Mindray.hh"

#include "Data/Volume.hh"

#include "glm/ext.hpp"

int main([[maybe_unused]] int argc, [[maybe_unused]] char **argv)
{
    std::ios::sync_with_stdio(false);

    using RButton = gui::Button<gui::Rectangle>;
    using Dropzone = gui::Dropzone<gui::Rectangle>;
    using Renderer = gui::Renderer<gui::Rectangle, data::Volume>;
    using Tree = gui::Tree<RButton, std::tuple<RButton>, std::tuple<RButton>>;

    using Instance = gui::Instance;
    using Window = gui::Window<std::tuple<RButton, Dropzone, Renderer, Tree>, std::tuple<opencl::ToPolar, opencl::ToCartesian, opencl::Slice, opencl::Threshold, opencl::Invert, opencl::Clamp, opencl::Contrast, opencl::Log2, opencl::Shrink, opencl::Fade, opencl::Sqrt, ultrasound::Mindray>>;

    Instance init;
    Window mainWindow(1024, 768);
    init.initGL();

    mainWindow.redraw();

    std::shared_ptr<ultrasound::Mindray> reader = std::make_shared<ultrasound::Mindray>();
    if (!reader->load("tests/data/1"))
    {
        std::terminate();
    }

    opencl::Device device;

    TTF_Font *font = init.loadFont("./res/fonts/cour.ttf");

    auto size = mainWindow.getSize();
    float wWidth = static_cast<float>(size.first);
    float wHeight = static_cast<float>(size.second);

    reader->load(device.context);

    std::shared_ptr<gui::Texture> t;

    // DROP ZONE

    gui::Rectangle dropRec(0.0f, wHeight / 2.0f, wWidth, wHeight / 2.0f);
    dropRec.texture->fill({0x3C, 0x3C, 0x3C, 0xFF});
    dropRec.update();
    auto dropzone = Dropzone::build(std::move(dropRec));
    mainWindow.addDrawable(std::shared_ptr(dropzone));

    // KERNELS

    int w = 1, h = 1;
    TTF_SizeText(font, "> KERNELS", &w, &h);
    t = std::make_shared<gui::Texture>(w + 2, h + 2);
    t->addText(font, "> KERNELS");

    auto alt = std::make_shared<gui::Texture>(*t);

    auto rec = RButton::build({wWidth - 2.0f * (static_cast<float>(w) + 2.0f), 0.0f, static_cast<float>(w) + 2.0f, static_cast<float>(h) + 2.0f, std::move(t)});
    rec->update();

    int oneWidth;
    TTF_SizeText(font, ">", &oneWidth, nullptr);

    alt->rotate(1, 1, oneWidth, oneWidth);

    auto tree = Tree::build(RButton({*rec}), std::move(alt));
    tree->x = wWidth - 2.0f * (static_cast<float>(w) + 2.0f);
    tree->w *= 2.0f;
    tree->y = 0.0f;
    tree->texture->fill({0x2C, 0x2C, 0x2C, 0xFF});
    tree->update();

    auto inputTree = Tree::build("INPUTS");
    auto outputTree = Tree::build("OUTPUTS");
    auto dataTree = Tree::build("DATA");

    tree->addBranch(std::shared_ptr(inputTree));
    tree->addBranch(std::shared_ptr(outputTree));
    tree->addBranch(std::shared_ptr(dataTree));

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

    auto mindray = gui::Kernel<ultrasound::Mindray>::buildButton<decltype(mainWindow.kernel), decltype(mainWindow.renderers), decltype(dropzone)>("MINDRAY", mainWindow.kernel, mainWindow.renderers, dropzone, reader);
    auto toPolar = gui::Kernel<opencl::ToPolar>::buildButton<decltype(mainWindow.kernel), decltype(mainWindow.renderers), decltype(dropzone)>("To Polar", mainWindow.kernel, mainWindow.renderers, dropzone, polar);
    auto toCartesian = gui::Kernel<opencl::ToCartesian>::buildButton<decltype(mainWindow.kernel), decltype(mainWindow.renderers), decltype(dropzone)>("To Cartesian", mainWindow.kernel, mainWindow.renderers, dropzone, cartesian);
    auto sliceK = gui::Kernel<opencl::Slice>::buildButton<decltype(mainWindow.kernel), decltype(mainWindow.renderers), decltype(dropzone)>("Slice", mainWindow.kernel, mainWindow.renderers, dropzone, slice);
    auto threshK = gui::Kernel<opencl::Threshold>::buildButton<decltype(mainWindow.kernel), decltype(mainWindow.renderers), decltype(dropzone)>("Threshold", mainWindow.kernel, mainWindow.renderers, dropzone, threshold);
    auto invK = gui::Kernel<opencl::Invert>::buildButton<decltype(mainWindow.kernel), decltype(mainWindow.renderers), decltype(dropzone)>("Invert", mainWindow.kernel, mainWindow.renderers, dropzone, invert);
    auto clampK = gui::Kernel<opencl::Clamp>::buildButton<decltype(mainWindow.kernel), decltype(mainWindow.renderers), decltype(dropzone)>("Clamp", mainWindow.kernel, mainWindow.renderers, dropzone, clamp);
    auto conK = gui::Kernel<opencl::Contrast>::buildButton<decltype(mainWindow.kernel), decltype(mainWindow.renderers), decltype(dropzone)>("Contrast", mainWindow.kernel, mainWindow.renderers, dropzone, contrast);
    auto logK = gui::Kernel<opencl::Log2>::buildButton<decltype(mainWindow.kernel), decltype(mainWindow.renderers), decltype(dropzone)>("Log2", mainWindow.kernel, mainWindow.renderers, dropzone, log);
    auto shrinkK = gui::Kernel<opencl::Shrink>::buildButton<decltype(mainWindow.kernel), decltype(mainWindow.renderers), decltype(dropzone)>("Shrink", mainWindow.kernel, mainWindow.renderers, dropzone, shrink);
    auto fadeK = gui::Kernel<opencl::Fade>::buildButton<decltype(mainWindow.kernel), decltype(mainWindow.renderers), decltype(dropzone)>("Fade", mainWindow.kernel, mainWindow.renderers, dropzone, fade);
    auto sqrtK = gui::Kernel<opencl::Sqrt>::buildButton<decltype(mainWindow.kernel), decltype(mainWindow.renderers), decltype(dropzone)>("Sqrt", mainWindow.kernel, mainWindow.renderers, dropzone, sqrt);

    inputTree->addLeaf(std::move(mindray));
    dataTree->addLeaf(std::move(toPolar));
    dataTree->addLeaf(std::move(toCartesian));
    dataTree->addLeaf(std::move(sliceK));
    dataTree->addLeaf(std::move(threshK));
    dataTree->addLeaf(std::move(invK));
    dataTree->addLeaf(std::move(clampK));
    dataTree->addLeaf(std::move(conK));
    dataTree->addLeaf(std::move(logK));
    dataTree->addLeaf(std::move(shrinkK));
    dataTree->addLeaf(std::move(fadeK));
    dataTree->addLeaf(std::move(sqrtK));

    mainWindow.addDrawable(std::shared_ptr(tree));

    // HIDE BUTTON

    std::shared_ptr<RButton> hideButton;
    t = std::make_shared<gui::Texture>(40, 40);
    t->fill({0x4E, 0x4E, 0x4E, 0xFF});
    hideButton = RButton::build({wWidth - 40.0f, wHeight - 20.0f, 40.0f, 20.0f, std::move(t)});
    hideButton->update();
    hideButton->onPress(
        [wptr = dropzone->weak_from_this(), wptr2 = tree->weak_from_this()]()
        {
            auto ptr = wptr.lock();
            auto ptr2 = wptr2.lock();
            ptr->hidden = !ptr->hidden;
            ptr2->hidden = !ptr2->hidden;
        });

    mainWindow.addDrawable(std::shared_ptr(hideButton));

    auto timeA = SDL_GetTicks();
    bool quit = false;

    std::size_t eventCount = 1;
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
                if (std::visit([](auto &&k)
                               { return k != nullptr; },
                               mainWindow.kernel))
                {
                    std::visit([e](auto &&k)
                               { k->eventManager.process(e); },
                               mainWindow.kernel);
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
                std::visit([&device](auto &&r)
                           {
                               if (r->tf->modified)
                               {
                                   r->tf->update();

                                   auto start = std::chrono::steady_clock::now();
                                   device.render(*r->tf);
                                   auto stop = std::chrono::steady_clock::now();
                                   std::cout << "Kernel Execution Time: " << std::chrono::duration_cast<std::chrono::duration<float, std::milli>>(stop - start).count() << "ms" << std::endl;

                                   r->texture->update(device.pixelBuffer);
                               }
                           },
                           renderer);
            }

            // Set volumes to finished processing
            for (auto &&renderer : mainWindow.renderers)
            {
                std::visit([&device](auto &&r)
                           { r->tf->modified = false; },
                           renderer);
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
