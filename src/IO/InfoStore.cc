#include "InfoStore.hh"

namespace io
{

    InfoStore::InfoStore(/* args */)
    {
    }

    InfoStore::~InfoStore()
    {
    }

    template <typename T>
    T &InfoStore::load(std::string &&str, T &&t)
    {
        return info.emplace(str, t);
    }

    template <typename T>
    T &InfoStore::fetch(std::string &str)
    {
        return info.at(str);
    }

    template bool &InfoStore::load<bool>(std::string &&str, bool &&t);
    template int8_t &InfoStore::load<int8_t>(std::string &&str, int8_t &&t);
    template int16_t &InfoStore::load<int16_t>(std::string &&str, int16_t &&t);
    template int32_t &InfoStore::load<int32_t>(std::string &&str, int32_t &&t);
    template uint8_t &InfoStore::load<uint8_t>(std::string &&str, uint8_t &&t);
    template uint16_t &InfoStore::load<uint16_t>(std::string &&str, uint16_t &&t);
    template uint32_t &InfoStore::load<uint32_t>(std::string &&str, uint32_t &&t);
    template float &InfoStore::load<float>(std::string &&str, float &&t);
    template double &InfoStore::load<double>(std::string &&str, double &&t);
    template InfoStore &InfoStore::load<InfoStore>(std::string &&str, InfoStore &&t);

    template bool &InfoStore::fetch<bool>(std::string &str);
    template int8_t &InfoStore::fetch<int8_t>(std::string &str);
    template int16_t &InfoStore::fetch<int16_t>(std::string &str);
    template int32_t &InfoStore::fetch<int32_t>(std::string &str);
    template uint8_t &InfoStore::fetch<uint8_t>(std::string &str);
    template uint16_t &InfoStore::fetch<uint16_t>(std::string &str);
    template uint32_t &InfoStore::fetch<uint32_t>(std::string &str);
    template float &InfoStore::fetch<float>(std::string &str);
    template double &InfoStore::fetch<double>(std::string &str);
    template InfoStore &InfoStore::fetch<InfoStore>(std::string &str);

} // namespace io
