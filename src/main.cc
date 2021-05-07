#include <iostream>
#include <cstddef>

#include "GUI/SDL2/Instance.hh"
#include "GUI/SDL2/Window.hh"

#include "IO/Ultrasound/Mindray.hh"

int main([[maybe_unused]] int argc, [[maybe_unused]] char** argv) {
    std::cout << "Hello, world!\n";

    io::Mindray reader;
    reader.load("tests/data/1/VirtualMachine.txt","tests/data/1/VirtualMachine.bin", "tests/data/1/BC_CinePartition0.bin");

    gui::Instance init();

    gui::Window mainWindow();
    
    return EXIT_SUCCESS;
}
