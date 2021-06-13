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
    reader.load("tests/data/2/VirtualMachine.txt", "tests/data/2/VirtualMachine.bin", "tests/data/2/BC_CinePartition0.bin");

    gui::Instance init;

    auto depth = reader.cpStore.fetch<int32_t>("Depth", 0);
    auto length = reader.cpStore.fetch<int32_t>("Length", 0);
    auto width = reader.cpStore.fetch<int32_t>("Width", 0);
    auto angleD = reader.cpStore.fetch<float>("AngleDelta", 0);

    std::vector<uint8_t> &data = reader.cpStore.fetch<uint8_t>("Data");
    
    Volume volume(depth, length, width, data);

    // const int numIterations = 50;

    gui::Window mainWindow(1024, 768);
    gui::Window &subWindow = mainWindow.subWindow(0, 0, 512, 512);
    
    subWindow.setActive();
    auto pair = subWindow.getSize();
    
    Device device;
    device.createDisplay(pair.first, pair.second, subWindow.glPixelBuffer);
    volume.sendToCl(device.context);
    device.prepareVolume(depth, length, width, angleD, volume.buffer);

    auto timeA = SDL_GetTicks();
    bool quit = false;
    while(!quit)
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
        }
        subWindow.setActive();
        subWindow.update();
        volume.update();
        device.render(volume.invMVTransposed.data(), subWindow.glPixelBuffer);
        subWindow.render();

        auto duration = long(timeA) + long(1000.0/60.0) - long(SDL_GetTicks());
        if (duration-2 > 0)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(duration-2)); // Sleep with 2ms left over.
        }
        while(SDL_GetTicks() < timeA + 1000/60); // Busy wait last 2 ms.
        timeA = SDL_GetTicks();
        
    }

    return EXIT_SUCCESS;
}
