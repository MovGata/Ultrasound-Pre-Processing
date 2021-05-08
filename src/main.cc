#include <iostream>
#include <cstddef>

#include "GUI/SDL2/Instance.hh"
#include "GUI/SDL2/Window.hh"

#include "IO/InfoStore.hh"
#include "IO/Ultrasound/Mindray.hh"

int main([[maybe_unused]] int argc, [[maybe_unused]] char** argv) {
    std::cout << "Hello, world!\n";

    io::Mindray reader;
    reader.load("tests/data/1/VirtualMachine.txt","tests/data/1/VirtualMachine.bin", "tests/data/1/BC_CinePartition0.bin");

    // std::cout << "InfoStore Test: " << reader.vmTxtStore.fetch<io::InfoStore>(std::string("CinePartition")).at(0).fetch<io::InfoStore>(std::string("CinePartition0")).at(0).fetch<uint32_t>(std::string("id_cine")).at(5);
    auto a = reader.vmTxtStore.fetch<io::InfoStore>(std::string("CinePartition"));
    auto b = a.at(0).fetch<io::InfoStore>(std::string("CinePartition0"));
    auto c = b.at(0).fetch<uint32_t>(std::string("id_cine"));
    std::cout << c.at(5) << std::endl;

    gui::Instance init();

    gui::Window mainWindow();
    
    return EXIT_SUCCESS;
}
