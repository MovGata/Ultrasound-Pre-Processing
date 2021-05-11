#ifndef IO_VECTORVARIANT_HH
#define IO_VECTORVARIANT_HH

#include <cctype>
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
    /*
     * @brief A class that contains a variadic variant of vector types.
     * 
     * @note Since std::vector<bool> is specialised and not useful here, boolean template parameters are internally represented by an aggregate type containing one bool.
     * 
     * @tparam T 
     */
    template <concepts::UniqueType... T>
        requires concepts::MindrayType<T...> && (!std::is_same<T, Bool>() && ...) class VectorVariant
    {
    private:
        template <typename U>
        using S = std::conditional_t<std::is_same_v<U, Bool>, uint8_t, U>;

        std::variant<std::vector<S<T>>...> var;

    public:
        VectorVariant() = default;
        ~VectorVariant() = default;

        template <concepts::SubType<T...> U>
        auto set(std::vector<S<U>> &&u) -> std::vector<S<U>> &;

        template <concepts::SubType<T...> U>
        auto set(S<U> &&u) -> U &;

        template <concepts::SubType<T...> U>
        auto get() -> std::vector<S<U>> &;

        template <concepts::SubType<T...> U>
        auto get(std::size_t i) -> U &;

        template <typename UnaryFunction>
        requires(concepts::CallableType<UnaryFunction, T> &&...) auto visit(UnaryFunction f) -> void;

        auto print(std::ostream &os, std::size_t depth) const -> void;
    };
} // namespace io

/*
 * @brief Templated Definitions
 * 
 */
namespace io
{
    template <concepts::UniqueType... T>
    template <concepts::SubType<T...> U>
    auto VectorVariant<T...>::set(std::vector<S<U>> &&u) -> std::vector<S<U>> &
    {
        return var.template emplace<std::vector<S<U>>>(std::forward<decltype(u)>(u));
    }

    template <concepts::UniqueType... T>
    template <concepts::SubType<T...> U>
    auto VectorVariant<T...>::set(S<U> &&u) -> U &
    {
        return reinterpret_cast<U &>(var.template emplace<std::vector<S<U>>>(std::vector<S<U>>({std::forward<decltype(u)>(u)})).at(0));
    }

    template <concepts::UniqueType... T>
    template <concepts::SubType<T...> U>
    auto VectorVariant<T...>::get() -> std::vector<S<U>> &
    {
        return std::get<std::vector<S<U>>>(var);
    }

    template <concepts::UniqueType... T>
    template <concepts::SubType<T...> U>
    auto VectorVariant<T...>::get(std::size_t i) -> U &
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
    template <concepts::UniqueType... T>
    template <typename UnaryFunction>
    requires(concepts::CallableType<UnaryFunction, T> &&...) auto VectorVariant<T...>::visit(UnaryFunction f) -> void
    {
        auto l = [f](auto &&v) {for (auto &e : v){f(e);} };
        std::visit(l, var);
    }

    template <concepts::UniqueType... T>
    auto VectorVariant<T...>::print(std::ostream &os, std::size_t depth) const -> void
    {
        std::visit(
            [&os, depth](auto &&a) {
                using type = std::remove_cvref<decltype(a)>;
                if constexpr (concepts::PrintableDepthType<type>)
                {
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