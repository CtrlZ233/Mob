#pragma once

#if defined(__GNUC) || defined (__clangd__)
    #define MOB_COMPILER_GCC_LIKE 1
#elif defined(_MSC_VER)
    #define MOB_COMPILER_MSVC 1
#endif

#if defined(__cplusplus)
    #if __cplusplus >= 202002L
        #define MOB_CPP20 1
    #elif __cplusplus >= 201703L
        #define MOB_CPP17 1
    #endif
#endif

namespace mob
{

namespace compiler
{

#if defined(MOB_COMPILER_GCC_LIKE)
    #define MOB_LIKELY_BUILTIN(expr) __builtin_expect(!!(expr), 1)
    #define MOB_UNLIKELY_BUILTIN(expr) __builtin_expect(!!(expr), 0)
#else
    #define MOB_LIKELY_BUILTIN(expr) (expr)
    #define MOB_UNLIKELY_BUILTIN(expr) (expr)
#endif

}

#if defined(MOB_CPP20)
    #define MOB_LIKELY_DEF(expr) (expr) [[likely]]
    #define MOB_UNLIKELY_DEF(expr) (expr) [[unlikely]]
#elif defined(MOB_COMPILER_GCC_LIKE)
    #define MOB_LIKELY_DEF(expr) mob::compiler::MOB_LIKELY_BUILTIN(expr)
    #define MOB_UNLIKELY_DEF(expr) mob::compiler::MOB_UNLIKELY_BUILTIN(expr)
#elif defined(MOB_COMPILER_MSVC) && defined(_HAS_CXX20) && _HAS_CXX20
    #define MOB_LIKELY_DEF(expr) (expr) [[likely]]
    #define MOB_UNLIKELY_DEF(expr) (expr) [[unlikely]]
#else
    #define MOB_LIKELY_DEF(expr) (expr)
    #define MOB_UNLIKELY_DEF(expr) (expr)
#endif

#define MOB_LIKELY(expr) MOB_LIKELY_DEF(expr)
#define MOB_UNLIKELY(expr) MOB_UNLIKELY_DEF(expr)

}