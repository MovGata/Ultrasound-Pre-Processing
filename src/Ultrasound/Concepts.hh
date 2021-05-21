#ifndef ULTRASOUND_CONCEPTS
#define ULTRASOUND_CONCEPTS

#include <cctype>
#include <string>

#include "../Concepts.hh"

namespace ultrasound
{
    class Mindray;

} // namespace ultrasound

namespace concepts
{

    template <typename T>
    concept UltrasoundType = SubType<T, ultrasound::Mindray>;

    template <typename... T>
    concept MindrayType = (SubType<T, bool, int8_t, int16_t, int32_t, uint8_t, uint16_t, uint32_t, float, double, std::string, std::size_t> && ...);

} // namespace concepts

#endif