/*
 * @file Mindray.hh
 * @author Harry Gougousidis (harry.gougousidis@outlook.com)
 * @version 0.1
 * @date 2021-05-10
 * @brief 
 */

#ifndef IO_ULTRASOUND_MINDRAY_HH
#define IO_ULTRASOUND_MINDRAY_HH

#include <array>
#include <cstddef>
#include <string>
#include <vector>

#include "../IO/InfoStore.hh"

namespace ultrasound
{
    
class Mindray
{
private:
public:
    Mindray(/* args */);
    ~Mindray();
    
    using vmBinInfoStore = io::InfoStore<bool, int8_t, int16_t, int32_t, uint8_t, uint16_t, uint32_t, float, double>;
    using vmTxtInfoStore = io::InfoStore<uint32_t, double, std::string>;
    using cpInfoStore = io::InfoStore<uint8_t, int32_t, std::size_t>;

    vmBinInfoStore vmBinStore;
    vmTxtInfoStore vmTxtStore;
    cpInfoStore cpStore;

    bool load(const char *vm_txt, const char *vm_bin, const char *cp);

};

} // namespace ultrasound

#endif