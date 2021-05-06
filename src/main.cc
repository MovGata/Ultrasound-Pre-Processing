#include <iostream>
#include <cstddef>

#include "GUI/SDL2/Instance.hh"
#include "GUI/SDL2/Window.hh"

#include "IO/Ultrasound/Mindray/Reader.hh"

int main([[maybe_unused]] int argc, [[maybe_unused]] char** argv) {
    std::cout << "Hello, world!\n";

    mindray::Reader reader("","");

    gui::Instance init();

    gui::Window mainWindow();
    
    return EXIT_SUCCESS;
}
