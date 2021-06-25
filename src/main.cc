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

    // const int numIterations = 50;
    gui::Window mainWindow(1024, 768);

    Device device;
    // gui::Window &subWindow = mainWindow.subWindow(0, 0, 0.5f, 0.5f);
    TTF_Font *font = init.loadFont("./res/fonts/cour.ttf");
    // subWindow.setActive();
    // auto pair = subWindow.getSize();
    {
        gui::Rectangle &dropRec = mainWindow.addRectangle(-1.0f, -1.0f, 1.0f, 0.5f);
        dropRec.setBG({0x4F, 0x7F, 0x4F, 0x4F});
    }

    mainWindow.addRectangle(0.0f, 0.0f, 0.25f, 0.5f);

    {
        gui::Rectangle &testText = mainWindow.addRectangle(0.5f, 0.8f, 0.25f, 0.1f);
        testText.setBG({0xFF, 0xFF, 0xFF, 0xFF});
        // testText.allocTexture(256, 38);
        testText.addText(font, "Add text test.");
    }

    {
        gui::Rectangle &rec = mainWindow.addRectangle(-1.0f, 0.0f, 0.5f, 0.5f);
        rec.allocTexture(1024, 1024);
        rec.setBG({0xFF, 0xFF, 0xFF, 0xFF});
    }

    mainWindow.rectangles.at(0).addCallback(gui::Rectangle::dropEventData, std::bind(gui::Rectangle::dropEvent, &mainWindow.rectangles.at(0), std::placeholders::_1));
    Volume &volume = mainWindow.rectangles.at(3).allocVolume(depth, length, width, data);

    device.createDisplay(1024, 1024, mainWindow.rectangles.at(3).pixelBuffer);
    volume.sendToCl(device.context);
    device.prepareVolume(depth, length, width, volume.buffer);

    auto timeA = SDL_GetTicks();
    bool quit = false;
    while (!quit)
    {
        SDL_Event e;
        while (SDL_PollEvent(&e))
        {
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

            // if (e.type >= SDL_USEREVENT)
            // {
            //     if (e.type == gui::Rectangle::dropEvent)
            //     {
            //         delete static_cast<float *>(e.user.data1); // Must do this as a consequence of SDL2 event lifetime.
            //         delete static_cast<float *>(e.user.data2); // It is safest to delete in the main loop, as an event may go through multiple handlers.
            //     }
            // }
        }
        mainWindow.setActive();

        // subWindow.setActive();
        // subWindow.update();
        volume.update();
        device.render(volume.invMVTransposed.data(), mainWindow.rectangles.at(3).pixelBuffer);
        // subWindow.render();
        mainWindow.update();
        mainWindow.render();

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
