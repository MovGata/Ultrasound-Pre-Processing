#include <iostream>
#include <cstddef>

#include "GUI/SDL2/Instance.hh"
#include "GUI/SDL2/Window.hh"

#include "IO/InfoStore.hh"
#include "Ultrasound/Mindray.hh"

int main([[maybe_unused]] int argc, [[maybe_unused]] char *argv[])
{
    std::ios::sync_with_stdio(false);

    std::cout << "Hello, world!\n";

    ultrasound::Mindray reader;
    reader.load("tests/data/1/VirtualMachine.txt", "tests/data/1/VirtualMachine.bin", "tests/data/1/BC_CinePartition0.bin");

    // std::cout << "InfoStore Test: " << reader.vmTxtStore.fetch<io::InfoStore>(std::string("CinePartition")).at(0).fetch<io::InfoStore>(std::string("CinePartition0")).at(0).fetch<uint32_t>(std::string("id_cine")).at(5);
    std::cout << "TEST" << std::endl;
    
    std::cout << reader.vmTxtStore << std::endl;
    
    std::cout << "ENDTEST" << std::endl;
    
    std::vector<uint16_t> x = reader.vmBinStore.fetch<uint16_t>(std::string("BDscPointRange"));
    std::vector<uint16_t> y = reader.vmBinStore.fetch<uint16_t>(std::string("BDscLineRange"));

    std::cout << "Image Size: (" << (x.at(0) > x.at(1) ? x.at(0) - x.at(1) : x.at(1) - x.at(0)) << ", " << (y.at(0) > y.at(1) ? y.at(0) - y.at(1) : y.at(1) - y.at(0)) << ")" << std::endl;

    gui::Instance init();

    gui::Window mainWindow();

    return EXIT_SUCCESS;
}
