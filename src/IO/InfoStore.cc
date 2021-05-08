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
    std::vector<T> &InfoStore::load(std::string &&str, std::vector<T> &&t)
    {
        auto pair = info.try_emplace(str, std::vector<T>());
        std::vector<T> &v = std::get<std::vector<T>>(pair.first->second);
        v.insert(v.end(), t.begin(), t.end());
        return v;
    }

    template <typename T>
    std::vector<T> &InfoStore::load(std::string &&str, T &&t)
    {
        auto pair = info.try_emplace(str, std::vector<T>());
        std::vector<T> &v = std::get<std::vector<T>>(pair.first->second);
        v.push_back(t);
        return v;
    }

    template <typename T>
    std::vector<T> &InfoStore::fetch(std::string &&str)
    {
        return std::get<std::vector<T>>(info.at(str));
    }

    template std::vector<bool> &InfoStore::load<bool>(std::string &&str, std::vector<bool> &&t);
    template std::vector<int8_t> &InfoStore::load<int8_t>(std::string &&str, std::vector<int8_t> &&t);
    template std::vector<int16_t> &InfoStore::load<int16_t>(std::string &&str, std::vector<int16_t> &&t);
    template std::vector<int32_t> &InfoStore::load<int32_t>(std::string &&str, std::vector<int32_t> &&t);
    template std::vector<uint8_t> &InfoStore::load<uint8_t>(std::string &&str, std::vector<uint8_t> &&t);
    template std::vector<uint16_t> &InfoStore::load<uint16_t>(std::string &&str, std::vector<uint16_t> &&t);
    template std::vector<uint32_t> &InfoStore::load<uint32_t>(std::string &&str, std::vector<uint32_t> &&t);
    template std::vector<float> &InfoStore::load<float>(std::string &&str, std::vector<float> &&t);
    template std::vector<double> &InfoStore::load<double>(std::string &&str, std::vector<double> &&t);
    template std::vector<std::string> &InfoStore::load<std::string>(std::string &&str, std::vector<std::string> &&t);
    template std::vector<InfoStore> &InfoStore::load<InfoStore>(std::string &&str, std::vector<InfoStore> &&t);

    template std::vector<bool> &InfoStore::load<bool>(std::string &&str, bool &&t);
    template std::vector<int8_t> &InfoStore::load<int8_t>(std::string &&str, int8_t &&t);
    template std::vector<int16_t> &InfoStore::load<int16_t>(std::string &&str, int16_t &&t);
    template std::vector<int32_t> &InfoStore::load<int32_t>(std::string &&str, int32_t &&t);
    template std::vector<uint8_t> &InfoStore::load<uint8_t>(std::string &&str, uint8_t &&t);
    template std::vector<uint16_t> &InfoStore::load<uint16_t>(std::string &&str, uint16_t &&t);
    template std::vector<uint32_t> &InfoStore::load<uint32_t>(std::string &&str, uint32_t &&t);
    template std::vector<float> &InfoStore::load<float>(std::string &&str, float &&t);
    template std::vector<double> &InfoStore::load<double>(std::string &&str, double &&t);
    template std::vector<std::string> &InfoStore::load<std::string>(std::string &&str, std::string &&t);
    template std::vector<InfoStore> &InfoStore::load<InfoStore>(std::string &&str, InfoStore &&t);

    template std::vector<bool> &InfoStore::fetch<bool>(std::string &&str);
    template std::vector<int8_t> &InfoStore::fetch<int8_t>(std::string &&str);
    template std::vector<int16_t> &InfoStore::fetch<int16_t>(std::string &&str);
    template std::vector<int32_t> &InfoStore::fetch<int32_t>(std::string &&str);
    template std::vector<uint8_t> &InfoStore::fetch<uint8_t>(std::string &&str);
    template std::vector<uint16_t> &InfoStore::fetch<uint16_t>(std::string &&str);
    template std::vector<uint32_t> &InfoStore::fetch<uint32_t>(std::string &&str);
    template std::vector<float> &InfoStore::fetch<float>(std::string &&str);
    template std::vector<double> &InfoStore::fetch<double>(std::string &&str);
    template std::vector<std::string> &InfoStore::fetch<std::string>(std::string &&str);
    template std::vector<InfoStore> &InfoStore::fetch<InfoStore>(std::string &&str);

} // namespace io
