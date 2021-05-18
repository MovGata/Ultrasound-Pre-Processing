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
    
    std::cout << reader.vmBinStore << std::endl;
    
    std::vector<uint16_t> x = reader.vmBinStore.fetch<uint16_t>(std::string("BDscPointRange"));
    std::vector<int16_t> y = reader.vmBinStore.fetch<int16_t>(std::string("BDscLineRange"));

    std::cout << "Image Size: (" << (x.at(0) > x.at(1) ? x.at(0) - x.at(1) : x.at(1) - x.at(0)) + 1 << ", " << (y.at(0) > y.at(1) ? y.at(0) - y.at(1) : y.at(1) - y.at(0)) + 1 << ")" << std::endl;

    gui::Instance init();

    gui::Window mainWindow();

    return EXIT_SUCCESS;
}
