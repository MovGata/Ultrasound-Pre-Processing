#ifndef IO_ULTRASOUND_MINDRAY_HH
#define IO_ULTRASOUND_MINDRAY_HH

#include <cstddef>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

namespace io
{
    
class Mindray
{
private:
    std::vector<std::byte> data;
    std::unordered_map<std::string, std::variant<bool, int8_t, int16_t, int32_t, uint8_t, uint16_t, uint32_t, float, double>> info;
public:
    Mindray(/* args */);
    ~Mindray();
};

} // namespace IO

#endif