#pragma once

#include <concepts>
#include <functional>
#include <tuple>
#include <type_traits>

namespace mob::task {

// Regular task wrapper using std::function
struct RegularTask {
    std::function<void()> func;

    void execute() {
        if (func) {
            func();
        }
    }
};

// Concept for latency-sensitive tasks
// Requires a void execute() or void execute() noexcept method
template<typename T>
concept ExecutableTask = requires(T task) {
    { task.execute() } -> std::same_as<void>;
};

// Type trait to check if a type is in a parameter pack
template<typename T, typename... Ts>
struct IsOneOf : std::disjunction<std::is_same<T, Ts>...> {};

template<typename T, typename... Ts>
inline constexpr bool isOneOf = IsOneOf<T, Ts...>::value;

// Helper to get index of type in parameter pack
template<typename T, typename... Ts>
struct IndexOf;

template<typename T, typename... Ts>
struct IndexOf<T, T, Ts...> : std::integral_constant<std::size_t, 0> {};

template<typename T, typename U, typename... Ts>
struct IndexOf<T, U, Ts...> : std::integral_constant<std::size_t, 1 + IndexOf<T, Ts...>::value> {};

template<typename T, typename... Ts>
inline constexpr std::size_t indexOf = IndexOf<T, Ts...>::value;

// Helper to get type at index in parameter pack
template<std::size_t I, typename... Ts>
struct TypeAt;

template<std::size_t I, typename T, typename... Ts>
struct TypeAt<I, T, Ts...> : TypeAt<I - 1, Ts...> {};

template<typename T, typename... Ts>
struct TypeAt<0, T, Ts...> {
    using type = T;
};

template<std::size_t I, typename... Ts>
using typeAt = typename TypeAt<I, Ts...>::type;

} // namespace mob::task
