#ifndef IO_INFOSTORE_HH
#define IO_INFOSTORE_HH

#include <memory>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

#include "Bool.hh"
#include "VectorVariant.hh"
#include "../Ultrasound/Concepts.hh"
#include "../Concepts.hh"

namespace io
{

    template <typename... T>
        requires concepts::UniqueType<T...> &&
        (!(std::same_as<T, Bool> || ...)) &&
        concepts::MindrayType<T...> class InfoStore
    {
    private:
        template <typename U>
        using S = std::conditional_t<std::is_same_v<U, bool>, Bool, U>;

    public:
        std::unordered_map<std::string, VectorVariant<T...>> info;
        InfoStore() = default;
        ~InfoStore() = default;

        template <typename U>
            requires concepts::SubType<U, T...> ||
            std::same_as<U, InfoStore<T...>> auto load(std::string &&str, std::vector<S<U>> &&t) -> std::vector<S<U>> &;

        template <typename U>
            requires concepts::SubType<U, T...> ||
            std::same_as<U, InfoStore<T...>> auto load(std::string &&str, U &&t) -> U &;

        template <typename U>
            requires concepts::SubType<U, T...> ||
            std::same_as<U, InfoStore<T...>> auto fetch(std::string &&str) -> std::vector<S<U>> &;

        template <typename U>
            requires concepts::SubType<U, T...> ||
            std::same_as<U, InfoStore<T...>> auto fetch(std::string &&str, std::size_t i) -> U &;

        template <typename UnaryFunction>
            requires(concepts::CallableType<UnaryFunction, T> &&...) &&
            concepts::CallableType<UnaryFunction, InfoStore<T...>> auto visit(std::string &&str, UnaryFunction f) -> void;

        auto print(std::ostream &os, std::size_t depth = 0) const -> void;

        friend auto operator<<<T...>(std::ostream &os, const InfoStore<T...> &is) -> std::ostream &;
    };

} // namespace io

namespace io
{
    template <typename... T>
        template <typename U>
        requires concepts::SubType<U, T...> ||
        std::same_as<U, InfoStore<T...>> auto InfoStore<T...>::load(std::string &&str, std::vector<S<U>> &&t) -> std::vector<S<U>> &
    {
        auto pair = info.try_emplace(std::forward<decltype(str)>(str), std::vector<S<U>>());
        return pair.first->second.template set<U>(std::forward<decltype(t)>(t));
    }

    template <typename... T>
        template <typename U>
        requires concepts::SubType<U, T...> ||
        std::same_as<U, InfoStore<T...>> auto InfoStore<T...>::load(std::string &&str, U &&t) -> U &
    {
        auto pair = info.try_emplace(std::forward<decltype(str)>(str), U());
        return pair.first->second.template set<U>(std::forward<decltype(t)>(t));
    }

    template <typename... T>
        template <typename U>
        requires concepts::SubType<U, T...> ||
        std::same_as<U, InfoStore<T...>> auto InfoStore<T...>::fetch(std::string &&str) -> std::vector<S<U>> &
    {
        return info.at(std::forward<std::string>(str)).template get<U>();
    }

    template <typename... T>
        template <typename U>
        requires concepts::SubType<U, T...> ||
        std::same_as<U, InfoStore<T...>> auto InfoStore<T...>::fetch(std::string &&str, std::size_t i) -> U &
    {
        return info.at(std::forward<std::string>(str)).template get<U>(i);
    }

    template <typename... T>
        template <typename UnaryFunction>
        requires(concepts::CallableType<UnaryFunction, T> &&...) &&
        concepts::CallableType<UnaryFunction, InfoStore<T...>> auto InfoStore<T...>::visit(std::string &&str, UnaryFunction f) -> void
    {
        f(fetch(std::forward<decltype(str)>(str)));
    }

    template <typename... T>
    auto InfoStore<T...>::print(std::ostream &os, std::size_t depth) const -> void
    {
        std::string indent(depth, '\t');
        for (auto &e : info)
        {
            os << indent << e.first << ":\n";
            e.second.print(os, depth + 1);
        }
    }

    template <typename... T>
    auto operator<<(std::ostream &os, const InfoStore<T...> &is) -> std::ostream &
    {
        is.print(os);
        return os;
    }

} // namespace io

#endif