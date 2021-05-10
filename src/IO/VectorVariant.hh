#ifndef IO_VECTORVARIANT_HH
#define IO_VECTORVARIANT_HH

#include <cctype>
#include <string>
#include <type_traits>
#include <variant>
#include <vector>

#include "../Ultrasound/Concepts.hh"
#include "../Concepts.hh"

/*
 * @brief Templated Declarations
 * 
 */
namespace io
{
    template <typename... T>
    requires concepts::MindrayType<T...> class VectorVariant
    {
    private:
        std::variant<std::vector<T>...> var;

    public:
        VectorVariant(/* args */);
        ~VectorVariant();

        template <concepts::SubType<T...> U>
        auto get() -> std::vector<U> &;

        template <typename UnaryFunction, concepts::SubType<T...> U>
        requires concepts::CallableType<UnaryFunction, U>
        auto visit(UnaryFunction f) -> void;
    };
} // namespace io

/*
 * @brief Templated Definitions
 * 
 */
namespace io
{

    template <typename... T>
    VectorVariant<T...>::VectorVariant()
    {
    }

    template <typename... T>
    VectorVariant<T...>::~VectorVariant()
    {
    }

    template <typename... T>
    template <concepts::SubType<T...> U>
    auto VectorVariant<T...>::get() -> std::vector<U> &
    {
        return std::get<std::vector<U>>(var);
    }

    template <typename ...T>
    template <typename UnaryFunction, concepts::SubType<T...> U>
    requires concepts::CallableType<UnaryFunction, U>
    auto VectorVariant<T...>::visit(UnaryFunction f) -> void
    {
        for (auto &v : std::get<U>(var))
        {
            f(v);
        }
    }

} // namespace io

#endif