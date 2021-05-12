#ifndef IO_VECTORVARIANT_HH
#define IO_VECTORVARIANT_HH

#include <cctype>
#include <iostream>
#include <string>
#include <type_traits>
#include <variant>
#include <vector>

#include "Bool.hh"
#include "Concepts.hh"
#include "../Ultrasound/Concepts.hh"
#include "../Concepts.hh"

/*
 * @brief Templated Declarations
 * 
 */
namespace io
{
    // Forward declaration of infostore (to avoid cyclic dependency).
    template <typename... T>
        requires concepts::UniqueType<T...> &&
        (!(std::same_as<T, Bool> || ...)) &&
        concepts::MindrayType<T...> class InfoStore;

    /*
     * @brief A class that contains a variadic variant of vector types.
     * 
     * @note Since std::vector<bool> is specialised and not useful here, boolean template parameters are internally represented by an aggregate type containing one bool.
     * 
     * @tparam T 
     */
    template <typename... T>
        requires concepts::UniqueType<T...> &&
        (!(std::same_as<T, Bool> || ...)) &&
        (concepts::MindrayType<T...>)class VectorVariant
    {
    private:
        template <typename U>
        using S = std::conditional_t<std::is_same_v<U, bool>, Bool, U>;

        std::variant<std::vector<InfoStore<T...>>, std::vector<S<T>>...> var;

    public:
        VectorVariant() = default;
        ~VectorVariant() = default;

        template <typename U>
            requires concepts::SubType<U, T...> ||
            std::same_as<U, InfoStore<T...>>
            VectorVariant(U &&u);

        template <typename U>
            requires(concepts::SubType<U, T...> && !std::same_as<U, bool>) ||
            std::same_as<U, InfoStore<T...>> VectorVariant(std::vector<U> &&u);

        template <typename U>
            requires concepts::SubType<U, T...> ||
            std::same_as<U, InfoStore<T...>> auto set(std::vector<S<U>> &&u) -> std::vector<S<U>> &;

        template <typename U>
            requires concepts::SubType<U, T...> ||
            std::same_as<U, InfoStore<T...>> auto set(U &&u) -> U &;

        template <typename U>
            requires concepts::SubType<U, T...> ||
            std::same_as<U, InfoStore<T...>> auto get() -> std::vector<S<U>> &;

        template <typename U>
            requires concepts::SubType<U, T...> ||
            std::same_as<U, InfoStore<T...>> auto get(std::size_t i) -> U &;

        template <typename UnaryFunction>
            requires(concepts::CallableType<UnaryFunction, T> &&...) &&
            concepts::CallableType<UnaryFunction, InfoStore<T...>> auto visit(UnaryFunction f) -> void;

        auto print(std::ostream &os, std::size_t depth) const -> void;
    };
} // namespace io

/*
 * @brief Templated Definitions
 * 
 */
namespace io
{
    template <typename... T>
        template <typename U>
        requires concepts::SubType<U, T...> ||
        std::same_as<U, InfoStore<T...>> VectorVariant<T...>::VectorVariant(U &&u) : var(std::vector<S<U>>({std::forward<S<U>>(reinterpret_cast<S<U> &&>(u))}))
    {
    }

    template <typename... T>
        template <typename U>
        requires(concepts::SubType<U, T...> && !std::same_as<U, bool>) ||
        std::same_as<U, InfoStore<T...>> VectorVariant<T...>::VectorVariant(std::vector<U> &&u) : var(std::forward<std::vector<S<U>>>(u))
    {
    }

    template <typename... T>
        template <typename U>
        requires concepts::SubType<U, T...> ||
        std::same_as<U, InfoStore<T...>> auto VectorVariant<T...>::set(std::vector<S<U>> &&u) -> std::vector<S<U>> &
    {
        return var.template emplace<std::vector<S<U>>>(std::forward<decltype(u)>(u));
    }

    template <typename... T>
        template <typename U>
        requires concepts::SubType<U, T...> ||
        std::same_as<U, InfoStore<T...>> auto VectorVariant<T...>::set(U &&u) -> U &
    {
        return reinterpret_cast<U &>(var.template emplace<std::vector<S<U>>>(std::vector<S<U>>({reinterpret_cast<S<U> &&>(std::forward<decltype(u)>(u))})).at(0));
    }

    template <typename... T>
        template <typename U>
        requires concepts::SubType<U, T...> ||
        std::same_as<U, InfoStore<T...>> auto VectorVariant<T...>::get() -> std::vector<S<U>> &
    {
        return std::get<std::vector<S<U>>>(var);
    }

    template <typename... T>
        template <typename U>
        requires concepts::SubType<U, T...> ||
        std::same_as<U, InfoStore<T...>> auto VectorVariant<T...>::get(std::size_t i) -> U &
    {
        return reinterpret_cast<U &>(std::get<std::vector<U>>(var).at(i));
    }

    /*
     * @brief Applies a function to every element in the vector in the variant.
     * 
     * @tparam T 
     * @tparam UnaryFunction 
     * @param f 
     */
    template <typename... T>
        template <typename UnaryFunction>
        requires(concepts::CallableType<UnaryFunction, T> &&...) &&
        concepts::CallableType<UnaryFunction, InfoStore<T...>> auto VectorVariant<T...>::visit(UnaryFunction f) -> void
    {
        auto l = [f](auto &&v) {for (auto &e : v){f(e);} };
        std::visit(l, var);
    }

    template <typename... T>
    auto VectorVariant<T...>::print(std::ostream &os, std::size_t depth) const -> void
    {
        std::visit(
            [&os, depth](auto &&a) {
                using type = std::remove_cvref<decltype(a.front())>;
                if constexpr (concepts::PrintableDepthType<type>)
                {
                    std::cout << "PRINTABLE" << std::endl;
                    for (auto &e : a)
                    {
                        e.print(os, depth);
                    }
                }
                else
                {
                    os << std::string(depth, '\t');
                    for (auto &e : a)
                    {
                        os << e << '\n';
                    }
                }
            },
            var);
    }

} // namespace io

#endif