#include <chrono>
#include <cstddef>
#include <iostream>
#include <thread>
#include <chrono>

#include "GUI/Instance.hh"
#include "GUI/Window.hh"

#include "OpenCL/Device.hh"

#include "IO/InfoStore.hh"
#include "Ultrasound/Mindray.hh"

#include "Data/Volume.hh"

int main([[maybe_unused]] int argc, [[maybe_unused]] char *argv[])
{
    std::ios::sync_with_stdio(false);

    ultrasound::Mindray reader;
    reader.load("tests/data/3/VirtualMachine.txt", "tests/data/3/VirtualMachine.bin", "tests/data/3/BC_CinePartition0.bin");

    gui::Instance init;

    auto depth = reader.cpStore.fetch<int32_t>("Depth", 0);
    auto length = reader.cpStore.fetch<int32_t>("Length", 0);
    auto width = reader.cpStore.fetch<int32_t>("Width", 0);
    // auto angleD = reader.cpStore.fetch<float>("AngleDelta", 0);

    std::cout << depth << ' ' << length << ' ' << width << std::endl;

    std::vector<uint8_t> &data = reader.cpStore.fetch<uint8_t>("Data");

    [[maybe_unused]] TTF_Font *font = init.loadFont("./res/fonts/cour.ttf");
    // const int numIterations = 50;
    gui::Window mainWindow(1024, 768);

    Device device;

    {
        std::shared_ptr<gui::Rectangle> dropRec = mainWindow.addRectangle(0.0f, -0.5f, 1.0f, 0.5f).lock();
        dropRec->setBG({0x20, 0x20, 0x20, 0xFF});
        dropRec->allocTexture(256, 38);
        dropRec->addCallback(SDL_MOUSEBUTTONDOWN, gui::Rectangle::moveStartEvent);

        dropRec->addCallback(gui::Rectangle::dropEventData, gui::Rectangle::dropEvent);
        dropRec->addCallback(gui::Rectangle::moveEventData, gui::Rectangle::stopEvent);
    }

    {
        std::shared_ptr<gui::Rectangle> pRec = mainWindow.addRectangle(0.25f, 0.5f, 0.25f, 0.5f).lock();
        pRec->setBG({0x2C, 0x2C, 0x2C, 0xFF});
        pRec->allocTexture(256, 38);
    }

    {
        std::shared_ptr<gui::Rectangle> testText = mainWindow.addRectangle(0.75f, 0.5f, 0.25f, 0.5f).lock();
        testText->setBG({0x25, 0x25, 0x25, 0xFF});
        testText->allocTexture(256, 38);

        testText->addCallback(SDL_MOUSEBUTTONDOWN, gui::Rectangle::dragStartEvent);
        testText->addCallback(SDL_MOUSEWHEEL, gui::Rectangle::scrollEvent);

        std::shared_ptr<gui::Rectangle> testElement = testText->addRectangle(0.0f, 0.8f, 1.0f, 0.2f).lock();
        testElement->setBG({0x00, 0x00, 0xFF, 0xFF});
        testElement->allocTexture(256, 38);
        testElement->addText(font, "Add text test.");
    }

    std::shared_ptr<Volume> volume;
    std::shared_ptr<gui::Rectangle> vRec;
    {
        std::shared_ptr<gui::Rectangle> rec = mainWindow.addRectangle(-0.5f, 0.5f, 0.5f, 0.5f).lock();
        rec->setBG({0x00, 0x00, 0x00, 0xFF});
        rec->allocTexture(1, 1);
        rec->addCallback(SDL_MOUSEBUTTONDOWN, gui::Rectangle::volumeStartEvent);

        vRec = rec->addRectangle(0.0f, 0.0f, 1.0f, 1.0f).lock();
        vRec->allocTexture(1024, 1024);
        volume = vRec->allocVolume(depth, length, width, data).lock();
    }

    device.createDisplay(1024, 1024, *vRec->pixelBuffer);
    volume->sendToCl(device.context);
    device.prepareVolume(depth, length, width, volume->buffer);

    auto timeA = SDL_GetTicks();
    bool quit = false;

    std::size_t eventCount = 1;
    while (!quit)
    {

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

        if (eventCount) // If no events have occurred, no changes to rendering have occurred.
        {
            mainWindow.setActive();

            volume->update();
            if (volume->modified)
            {
                device.render(volume->invMVTransposed.data(), *vRec->pixelBuffer);
            }

            mainWindow.update();
            mainWindow.render();
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
