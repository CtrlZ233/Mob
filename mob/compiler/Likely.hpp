// Likely.hpp
#pragma once

// ==================== Compiler Detection ====================
#if defined(__GNUC__) || defined(__clang__)
    #define MOB_COMPILER_GCC_LIKE 1
#elif defined(_MSC_VER)
    #define MOB_COMPILER_MSVC 1
#endif

// ==================== C++ Standard Detection ====================
#if defined(__cplusplus)
    #if __cplusplus >= 202002L
        #define MOB_CPP20 1
    #elif __cplusplus >= 201703L
        #define MOB_CPP17 1
    #endif
#endif

// MSVC-specific C++20 detection
#if defined(_MSC_VER) && defined(_HAS_CXX20) && _HAS_CXX20
    #define MOB_MSVC_CPP20 1
#endif

// ==================== Core Implementation ====================
// Expression-level hints (can be used in any expression context)
// These use __builtin_expect for maximum compatibility
#if defined(MOB_COMPILER_GCC_LIKE)
    // GCC/Clang builtin functions - works in all contexts
    #define MOB_LIKELY(expr) (__builtin_expect(!!(expr), 1))
    #define MOB_UNLIKELY(expr) (__builtin_expect(!!(expr), 0))
#else
    // Fallback implementation (no optimization)
    #define MOB_LIKELY(expr) (expr)
    #define MOB_UNLIKELY(expr) (expr)
#endif

// ==================== Statement-level Attributes (C++20) ====================
// These use [[likely]]/[[unlikely]] attributes for better semantics
#if defined(MOB_CPP20) || defined(MOB_MSVC_CPP20)
    #define MOB_LIKELY_BRANCH [[likely]]
    #define MOB_UNLIKELY_BRANCH [[unlikely]]
#else
    #define MOB_LIKELY_BRANCH
    #define MOB_UNLIKELY_BRANCH
#endif

// ==================== Helper Macros ====================
// Conditional statement helpers
#define MOB_IF_LIKELY(expr) if (MOB_LIKELY(expr))
#define MOB_IF_UNLIKELY(expr) if (MOB_UNLIKELY(expr))

// Loop helpers
#define MOB_WHILE_LIKELY(expr) while (MOB_LIKELY(expr))
#define MOB_WHILE_UNLIKELY(expr) while (MOB_UNLIKELY(expr))

// Switch statement helpers (C++20)
#if defined(MOB_CPP20) || defined(MOB_MSVC_CPP20)
    #define MOB_CASE_LIKELY [[likely]] case
    #define MOB_CASE_UNLIKELY [[unlikely]] case
#else
    #define MOB_CASE_LIKELY case
    #define MOB_CASE_UNLIKELY case
#endif

// ==================== Advanced Usage Patterns ====================
// For if-else with branch hints
#define MOB_IF_LIKELY_ELSE(expr) \
    if (MOB_LIKELY(expr)) MOB_LIKELY_BRANCH

#define MOB_IF_UNLIKELY_ELSE(expr) \
    if (MOB_UNLIKELY(expr)) MOB_UNLIKELY_BRANCH

// ==================== Documentation ====================
/*
 * Usage Guide:
 *
 * 1. Expression-level hints (use in any expression):
 *    if (MOB_LIKELY(ptr != nullptr)) { ... }
 *    while (MOB_UNLIKELY(error)) { ... }
 *
 * 2. Statement-level attributes (C++20, better semantics):
 *    if (condition) MOB_LIKELY_BRANCH { ... }
 *    else MOB_UNLIKELY_BRANCH { ... }
 *
 * 3. Helper macros (recommended for readability):
 *    MOB_IF_LIKELY(condition) { ... }
 *    MOB_IF_UNLIKELY(condition) { ... }
 *
 * 4. Switch statements (C++20):
 *    switch (value) {
 *        MOB_CASE_LIKELY 1: ...
 *        MOB_CASE_UNLIKELY 2: ...
 *    }
 *
 * Performance Tips:
 * - Use LIKELY for hot paths (>90% probability)
 * - Use UNLIKELY for error handling (<10% probability)
 * - Don't overuse - only for critical performance paths
 * - Profile to verify effectiveness
 */