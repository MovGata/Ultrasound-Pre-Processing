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
    gui::Window<gui::Button<gui::Rectangle>, gui::Dropzone<gui::Rectangle>, gui::Renderer<gui::Rectangle>, gui::Tree<gui::Button<gui::Rectangle>, std::tuple<gui::Button<gui::Rectangle>>, std::tuple<gui::Button<gui::Rectangle>>>> mainWindow(1024, 768);
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

    // int fontHeight = TTF_FontHeight(font) + 1; // +1 so text doesn't touch

    // auto size = mainWindow.getSize();
    // float fontRatio = static_cast<float>(fontHeight) / static_cast<float>(size.second);

    // {
    //     std::shared_ptr<gui::Rectangle> dropRec = mainWindow.addRectangle(0.0f, -0.5f, 1.0f, 0.5f).lock();
    //     dropRec->setBG({0x20, 0x20, 0x20, 0xFF});
    //     dropRec->allocTexture(256, 38);
    //     // dropRec->addCallback(gui::Rectangle::dropEventData, gui::Rectangle::dropEvent);
    //     // dropRec->addCallback(gui::Rectangle::moveEventData, gui::Rectangle::stopEvent);
    // }

    // {
    //     std::shared_ptr<gui::Rectangle> pRec = mainWindow.addRectangle(0.25f, 0.5f, 0.25f, 0.5f).lock();
    //     pRec->setBG({0x2C, 0x2C, 0x2C, 0xFF});
    //     pRec->allocTexture(256, 38);

    //     glm::mat4 scale = glm::scale(glm::mat4(1.0f), {1.0f, fontRatio, 1.0f});
    //     scale = glm::inverse(pRec->modelview) * scale;
    //     glm::mat4 pixels = glm::scale(glm::mat4(1.0f), {1.0f, scale[1][1], 1.0f});
    //     pixels = pRec->modelview * pixels;

    //     std::shared_ptr<gui::Rectangle> program;
    //     auto itr = device.programs.begin();
    //     for (std::size_t i = 0; i < device.programs.size(); ++i)
    //     {
    //         program = pRec->addRectangle(0.0f, 1.0f - scale[1][1] - 2.0f * scale[1][1] * static_cast<float>(i), 1.0f, scale[1][1]).lock();
    //         program->setBG({0x3C, 0x3C, 0x3C, 0xFF});

    //         // program = program->addRectangle(, 0.0f, ,1.0f);
    //         program->allocTexture(static_cast<int>(pixels[0][0] * static_cast<float>(size.first)), static_cast<int>(pixels[1][1] * static_cast<float>(size.second)));
    //         program->addText(font, itr->first);
    //         // program->addCallback(SDL_MOUSEBUTTONDOWN, gui::Rectangle::hideEvent);
    //         ++itr;
    //     }
    // }

    // {
    //     auto itr = device.programs.begin();
    //     for (std::size_t i = 0; i < device.programs.size(); ++i)
    //     {
    //         std::shared_ptr<gui::Rectangle> kernels = mainWindow.addRectangle(0.75f, 0.5f, 0.25f, 0.5f).lock();
    //         kernels->setBG({0x25, 0x25, 0x25, 0xFF});
    //         kernels->allocTexture(1, 1);
    //         // kernels->addCallback(gui::Rectangle::showEventData, gui::Rectangle::showEvent);
    //         // kernels->addCallback(SDL_MOUSEWHEEL, gui::Rectangle::scrollEvent);
    //         kernels->hidden = false;
    //         kernels->text = itr->first;

    //         glm::mat4 scale = glm::scale(glm::mat4(1.0f), {1.0f, fontRatio, 1.0f});
    //         scale = glm::inverse(kernels->modelview) * scale;
    //         glm::mat4 pixels = glm::scale(glm::mat4(1.0f), {1.0f, scale[1][1], 1.0f});
    //         pixels = kernels->modelview * pixels;

    //         std::shared_ptr<gui::Rectangle> kernel;
    //         auto kItr = itr->second.kernels.begin();
    //         for (std::size_t j = 0; j < itr->second.kernels.size(); ++j)
    //         {
    //             kernel = kernels->addRectangle(0.0f, 1.0f - scale[1][1] - 2.0f * scale[1][1] * static_cast<float>(j), 1.0f, scale[1][1]).lock();
    //             kernel->setBG({0x35, 0x35, 0x35, 0xFF});
    //             kernel->allocTexture(static_cast<int>(pixels[0][0] * static_cast<float>(size.first)), static_cast<int>(pixels[1][1] * static_cast<float>(size.second)));
    //             kernel->addText(font, kItr->first);
    //             // kernel->addCallback(SDL_MOUSEBUTTONDOWN, gui::Rectangle::dragStartEvent);
    //             ++kItr;
    //         }
    //         ++itr;
    //     }
    // }

    // std::shared_ptr<gui::Rectangle> vRec = mainWindow.addRectangle(-0.5f, 0.5f, 0.5f, 0.5f).lock();

    auto size = mainWindow.getSize();
    float wWidth = static_cast<float>(size.first);
    float wHeight = static_cast<float>(size.second);

    // RENDERER

    std::shared_ptr<data::Volume> volume = std::make_shared<data::Volume>(depth, length, width, data);
    std::shared_ptr<gui::Texture> t = std::make_shared<gui::Texture>(512, 512);
    std::shared_ptr<gui::Renderer<gui::Rectangle>> vRec = gui::Renderer<gui::Rectangle>::build({(wWidth - std::max(wWidth, wHeight)) / 2.0f, (wHeight - std::max(wWidth, wHeight)) / 2.0f, std::max(wWidth, wHeight), std::max(wWidth, wHeight), std::move(t)});
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
    TTF_SizeText(font, "TEST TEXT", &w, &h);

    t = std::make_shared<gui::Texture>(w + 2, h + 2);
    t->addText(font, "TEST TEXT");

    auto rec = gui::Button<gui::Rectangle>::build({wWidth - (static_cast<float>(w) + 2.0f), 0.0f, static_cast<float>(w) + 2.0f, static_cast<float>(h) + 2.0f, std::move(t)});
    rec->update();

    auto tree = gui::Tree<gui::Button<gui::Rectangle>, std::tuple<gui::Button<gui::Rectangle>>, std::tuple<gui::Button<gui::Rectangle>>>::build(gui::Button<gui::Rectangle>({*rec}));
    tree->x = wWidth - (static_cast<float>(w) + 2.0f);
    tree->y = 0.0f;
    tree->w = static_cast<float>(w) + 2.0f;
    tree->h = wHeight / 2.0f;
    tree->texture->fill({0x2C, 0x2C, 0x2C, 0xFF});
    tree->update();
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

        if (eventCount) // If no events have occurred, no changes to drawing have occurred.
        {
            // mainWindow.setActive();

            volume->update();
            if (volume->modified)
            {
                device.render(volume->invMVTransposed.data());
                vRec->texture->update(device.pixelBuffer);
            }

            mainWindow.update();

            mainWindow.clean();
            mainWindow.draw();
        }

        SDL_Event e;
        eventCount = 0;
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
