#ifndef IO_INFOSTORE_HH
#define IO_INFOSTORE_HH

#include <memory>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

namespace io
{

class InfoStore
{
private:
    std::unordered_map<std::string, std::variant<
        std::vector<bool>,
        std::vector<int8_t>, std::vector<int16_t>, std::vector<int32_t>,
        std::vector<uint8_t>, std::vector<uint16_t>, std::vector<uint32_t>,
        std::vector<float>, std::vector<double>,
        std::vector<std::string>,
        std::vector<InfoStore>
    >> info;
public:
    InfoStore(/* args */);
    ~InfoStore();

    template<typename T>
    std::vector<T> &load(std::string &&str, std::vector<T> &&t);

    template<typename T>
    std::vector<T> &load(std::string &&str, T &&t);

    template<typename T>
    std::vector<T> &fetch(std::string &&str);


};



} // namespace io

#endif