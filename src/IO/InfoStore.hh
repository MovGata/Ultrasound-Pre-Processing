#ifndef IO_INFOSTORE_HH
#define IO_INFOSTORE_HH

#include <memory>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

#include "VectorVariant.hh"
#include "../Ultrasound/Concepts.hh"
#include "../Concepts.hh"

namespace io
{

    template <typename... T>
    requires concepts::MindrayType<T...> class InfoStore
    {
    private:
    public:
        std::unordered_map<std::string, VectorVariant<InfoStore<T...>, T...>> info;
        InfoStore() = default;
        ~InfoStore() = default;

        template <concepts::SubType<InfoStore<T...>, T...> U>
        std::vector<U> &load(std::string &&str, std::vector<U> &&t);

        template <concepts::SubType<InfoStore<T...>, T...> U>
        std::vector<U> &load(std::string &&str, U &&t);

        template <concepts::SubType<InfoStore<T...>, T...> U>
        std::vector<U> &fetch(std::string &&str);

        template <typename UnaryFunction, concepts::SubType<InfoStore<T...>, T...> U>
        requires concepts::CallableType<UnaryFunction, U>
        auto visit(std::string &&str, UnaryFunction f) -> void;
    };

} // namespace io

namespace io
{
    template <typename... T>
    template <concepts::SubType<InfoStore<T...>, T...> U>
    std::vector<U> &InfoStore<T...>::load(std::string &&str, std::vector<U> &&t)
    {
        auto pair = info.try_emplace(std::forward<decltype(str)>(str), std::vector<U>());
        std::vector<U> &v = std::get<std::vector<U>>(pair.first->second);
        v.insert(v.end(), std::make_move_iterator(t.begin()), std::make_move_iterator(t.end()));
        return v;
    }

    template <typename... T>
    template <concepts::SubType<InfoStore<T...>, T...> U>
    std::vector<U> &InfoStore<T...>::load(std::string &&str, U &&t)
    {
        auto pair = info.try_emplace(std::forward<decltype(str)>(str), std::vector<U>());
        std::vector<U> &v = std::get<std::vector<U>>(pair.first->second);
        v.push_back(std::forward<U>(t));
        return v;
    }

    template <typename... T>
    template <concepts::SubType<InfoStore<T...>, T...> U>
    std::vector<U> &InfoStore<T...>::fetch(std::string &&str)
    {
        return info.at(std::forward<decltype(str)>(str)).get<U>();
    }

    template <typename... T>
    template <typename UnaryFunction, concepts::SubType<InfoStore<T...>, T...> U>
    requires concepts::CallableType<UnaryFunction, U>
    auto InfoStore<T...>::visit(std::string &&str, UnaryFunction f) -> void
    {
        f(fetch(std::forward<decltype(str)>(str)));
    }

} // namespace io

#endif