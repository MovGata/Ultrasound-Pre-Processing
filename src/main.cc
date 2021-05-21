#include <iostream>
#include <cstddef>

#include "GUI/SDL2/Instance.hh"
#include "GUI/SDL2/Window.hh"

#include "IO/InfoStore.hh"
#include "Ultrasound/Mindray.hh"

int main([[maybe_unused]] int argc, [[maybe_unused]] char *argv[])
{
    std::ios::sync_with_stdio(false);

    ultrasound::Mindray reader;
    reader.load("tests/data/1/VirtualMachine.txt", "tests/data/1/VirtualMachine.bin", "tests/data/1/BC_CinePartition0.bin");
        
    gui::Instance init;

    gui::Window mainWindow;

    mainWindow.update();

    SDL_Delay(2000);

    return EXIT_SUCCESS;
}
