#ifndef CONCEPTS_HH
#define CONCEPTS_HH

#include <concepts>
#include <type_traits>

namespace concepts
{
    template<typename Callable, typename ...Args>
    concept CallableType = std::invocable<Callable, Args...>;
    
    template<typename T, typename ...U>
    concept SubType = (std::same_as<T, U> || ...);
   
    template <typename T>
    constexpr bool are_unique() { return true; }

    template <typename T, typename ...U>
    constexpr bool are_unique() { return (!SubType<T, U...> && are_unique<U...>()); }
    
    template<typename ...T>
    concept UniqueType = requires {are_unique<T...>();};

} // namespace concepts


#endif