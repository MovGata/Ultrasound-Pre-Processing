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

#include "IO/InfoStore.hh"
#include "Ultrasound/Mindray.hh"

#include "Data/Volume.hh"

#include "glm/ext.hpp"

int main([[maybe_unused]] int argc, [[maybe_unused]] char *argv[])
{
    std::ios::sync_with_stdio(false);

    gui::Instance init;
    gui::Window<gui::Button<gui::Rectangle>, gui::Dropzone<gui::Rectangle>, gui::Renderer<gui::Rectangle, data::Volume>, gui::Tree<gui::Button<gui::Rectangle>, std::tuple<gui::Button<gui::Rectangle>>, std::tuple<gui::Button<gui::Rectangle>>>> mainWindow(1024, 768);
    init.initGL();

    mainWindow.redraw();

    ultrasound::Mindray reader;
    reader.load("tests/data/3/VirtualMachine.txt", "tests/data/3/VirtualMachine.bin", "tests/data/3/BC_CinePartition0.bin");

    auto depth = reader.cpStore.fetch<int32_t>("Depth", 0);
    auto length = reader.cpStore.fetch<int32_t>("Length", 0);
    auto width = reader.cpStore.fetch<int32_t>("Width", 0);
    // auto angleD = reader.cpStore.fetch<float>("AngleDelta", 0);

    std::cout << depth << ' ' << length << ' ' << width << std::endl;

    std::vector<uint8_t> &data = reader.cpStore.fetch<uint8_t>("Data");

    [[maybe_unused]] TTF_Font *font = init.loadFont("./res/fonts/cour.ttf");
    // const int numIterations = 50;

    Device device;

    auto size = mainWindow.getSize();
    float wWidth = static_cast<float>(size.first);
    float wHeight = static_cast<float>(size.second);

    // RENDERER

    std::shared_ptr<data::Volume> volume = std::make_shared<data::Volume>(depth, length, width, data);
    std::shared_ptr<gui::Texture> t = std::make_shared<gui::Texture>(512, 512);
    std::shared_ptr<gui::Renderer<gui::Rectangle, data::Volume>> vRec = gui::Renderer<gui::Rectangle, data::Volume>::build({(wWidth - std::max(wWidth, wHeight)) / 2.0f, (wHeight - std::max(wWidth, wHeight)) / 2.0f, std::max(wWidth, wHeight), std::max(wWidth, wHeight), std::move(t)}, std::shared_ptr(volume));
    vRec->update();
    mainWindow.addDrawable(std::shared_ptr(vRec));

    // DROP ZONE

    gui::Rectangle dropRec(0.0f, wHeight / 2.0f, wWidth, wHeight / 2.0f);
    dropRec.texture->fill({0x3C, 0x3C, 0x3C, 0xFF});
    dropRec.update();
    std::shared_ptr<gui::Dropzone<gui::Rectangle>> dropzone = gui::Dropzone<gui::Rectangle>::build(std::move(dropRec));
    mainWindow.addDrawable(std::shared_ptr(dropzone));

    // KERNELS

    int w = 1, h = 1;
    TTF_SizeText(font, "KERNELS", &w, &h);
    t = std::make_shared<gui::Texture>(w + 2, h + 2);
    t->addText(font, "KERNELS");
    auto rec = gui::Button<gui::Rectangle>::build({wWidth - (static_cast<float>(w) + 2.0f), 0.0f, static_cast<float>(w) + 2.0f, static_cast<float>(h) + 2.0f, std::move(t)});
    rec->update();

    auto tree = gui::Tree<gui::Button<gui::Rectangle>, std::tuple<gui::Button<gui::Rectangle>>, std::tuple<gui::Button<gui::Rectangle>>>::build(gui::Button<gui::Rectangle>({*rec}));
    tree->x = wWidth - (static_cast<float>(w) + 2.0f);
    tree->y = 0.0f;
    tree->texture->fill({0x2C, 0x2C, 0x2C, 0xFF});
    tree->update();

    auto pitr = device.programs.begin();
    for (std::size_t i = 0; i < device.programs.size(); ++i)
    {
        w = 1;
        h = 1;
        TTF_SizeText(font, pitr->first.c_str(), &w, &h);
        t = std::make_shared<gui::Texture>(w + 2, h + 2);
        t->addText(font, pitr->first);

        rec = gui::Button<gui::Rectangle>::build({wWidth - (static_cast<float>(w) + 2.0f), 0.0f, static_cast<float>(w) + 2.0f, static_cast<float>(h) + 2.0f, std::move(t)});
        rec->update();

        auto program = gui::Tree<gui::Button<gui::Rectangle>, std::tuple<gui::Button<gui::Rectangle>>, std::tuple<gui::Button<gui::Rectangle>>>::build(gui::Button<gui::Rectangle>({*rec}));

        tree->addBranch(std::shared_ptr(program));

        std::shared_ptr<gui::Button<gui::Rectangle>> kernel;
        auto kitr = pitr->second.kernels.begin();
        for (std::size_t j = 0; j < pitr->second.kernels.size(); ++j)
        {
            w = 1;
            h = 1;
            TTF_SizeText(font, kitr->first.c_str(), &w, &h);
            t = std::make_shared<gui::Texture>(w + 2, h + 2);
            t->addText(font, kitr->first);

            kernel = gui::Button<gui::Rectangle>::build({0.0f, 0.0f, static_cast<float>(w) + 2.0f, static_cast<float>(h) + 2.0f, std::move(t)});

            program->addLeaf(std::move(kernel));

            ++kitr;
        }

        ++pitr;
    }

    mainWindow.addDrawable(std::shared_ptr(tree));

    // HIDE BUTTON

    std::shared_ptr<gui::Button<gui::Rectangle>> hideButton;
    t = std::make_shared<gui::Texture>(40, 40);
    t->fill({0x4E, 0x4E, 0x4E, 0xFF});
    hideButton = gui::Button<gui::Rectangle>::build({wWidth - 40.0f, wHeight - 20.0f, 40.0f, 20.0f, std::move(t)});
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

    device.createDisplay(512, 512);

    volume->sendToCl(device.context);
    device.prepareVolume(depth, length, width, volume->buffer);

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
                mainWindow.process(e);
            }
        }

        if (eventCount) // If no events have occurred, no changes to drawing have occurred.
        {
            // mainWindow.setActive();

            if (volume->modified)
            {
                volume->update();
                device.render(volume->invMVTransposed.data());
                vRec->texture->update(device.pixelBuffer);
            }

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
