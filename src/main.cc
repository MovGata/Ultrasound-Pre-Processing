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
    auto angleD = reader.cpStore.fetch<float>("AngleDelta", 0);

    std::cout << depth << ' ' << length << ' ' << width << std::endl;

    std::vector<uint8_t> &data = reader.cpStore.fetch<uint8_t>("Data");


    // const int numIterations = 50;
    gui::Window mainWindow(1024, 768);

    Device device;
    // gui::Window &subWindow = mainWindow.subWindow(0, 0, 0.5f, 0.5f);

    // subWindow.setActive();
    // auto pair = subWindow.getSize();
    mainWindow.addRectangle(-1.0f, -1.0f, 1.0f, 1.0f);
    mainWindow.addRectangle(0.0f, 0.0f, 0.25f, 0.5f);
    mainWindow.addRectangle(0.25f, 0.0f, 0.25f, 0.5f);

    
    gui::Rectangle &rec = mainWindow.addRectangle(-1.0f, 0.0f, 0.5f, 0.5f);
    rec.allocTexture(512, 512);
    Volume &volume = rec.allocVolume(depth, length, width, data);
    device.createDisplay(512, 512, rec.pixelBuffer);
    volume.sendToCl(device.context);
    device.prepareVolume(depth, length, width, angleD, volume.buffer);

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
        }
        mainWindow.setActive();

        // subWindow.setActive();
        // subWindow.update();
        volume.update();
        device.render(volume.invMVTransposed.data(), rec.pixelBuffer);
        // subWindow.render();
        mainWindow.update();
        mainWindow.render();

        auto duration = long(timeA) + long(1000.0 / 60.0) - long(SDL_GetTicks());
        if (duration - 2 > 0)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(duration - 2)); // Sleep with 2ms left over.
        }
        while (SDL_GetTicks() < timeA + 1000 / 60)
            ; // Busy wait last 2 ms.
        timeA = SDL_GetTicks();
    }

    return EXIT_SUCCESS;
}
