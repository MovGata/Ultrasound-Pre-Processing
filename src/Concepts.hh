#ifndef CONCEPTS_HH
#define CONCEPTS_HH

#include <type_traits>

namespace concepts
{
    
    template <typename T, typename... U>
    constexpr bool is_any() { return std::disjunction<std::is_same<T, U>...>::value; }
 
    template<typename Callable, typename ...Args>
    constexpr bool is_callable() { return (std::is_function<Callable>::value && std::is_invocable<Callable, Args...>::value); }

    template<typename Callable, typename ...Args>
    concept CallableType = is_callable<Callable, Args...>();
    
    template<typename T, typename ...U>
    concept SubType = is_any<T, U...>();
   
    template <typename T>
    constexpr bool are_unique() { return true; }

    template <typename T, typename ...U>
    constexpr bool are_unique() { return ((!SubType<T, U...>) && are_unique<U...>); }
    
    template<typename ...T>
    concept UniqueType = are_unique<T...>();


} // namespace concepts


#endif