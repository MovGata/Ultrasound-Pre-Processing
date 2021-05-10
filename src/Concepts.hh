#ifndef CONCEPTS_HH
#define CONCEPTS_HH

#include <type_traits>

namespace concepts
{
    
    template <typename T, typename... U>
    constexpr bool is_any() { return std::disjunction<std::is_same<T, U>...>(); }
    
    template<typename Callable, typename ...Args>
    constexpr bool is_callable() { return (std::is_function<Callable>() && std::is_invocable<Callable, Args...>()); }

    template<typename Callable, typename ...Args>
    concept CallableType = is_callable<Callable, Args...>();
    
    template<typename T, typename ...U>
    concept SubType = is_any<T, U...>();

} // namespace concepts


#endif