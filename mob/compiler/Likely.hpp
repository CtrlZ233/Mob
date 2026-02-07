// Likely.hpp
#pragma once

// ==================== 编译器检测 ====================
#if defined(__GNUC__) || defined(__clang__)
    #define MOB_COMPILER_GCC_LIKE 1
#elif defined(_MSC_VER)
    #define MOB_COMPILER_MSVC 1
#endif

// ==================== C++ 标准检测 ====================
#if defined(__cplusplus)
    #if __cplusplus >= 202002L
        #define MOB_CPP20 1
    #elif __cplusplus >= 201703L
        #define MOB_CPP17 1
    #endif
#endif

// MSVC 特定的 C++20 检测
#if defined(_MSC_VER) && defined(_HAS_CXX20) && _HAS_CXX20
    #define MOB_MSVC_CPP20 1
#endif

// ==================== 核心实现 ====================
#if defined(MOB_CPP20) || defined(MOB_MSVC_CPP20)
    // C++20 标准属性 - 最简洁的语法
    #define MOB_LIKELY(expr) (expr) [[likely]]
    #define MOB_UNLIKELY(expr) (expr) [[unlikely]]
    
#elif defined(MOB_COMPILER_GCC_LIKE)
    // GCC/Clang 内置函数
    #define MOB_LIKELY(expr) (__builtin_expect(!!(expr), 1))
    #define MOB_UNLIKELY(expr) (__builtin_expect(!!(expr), 0))
    
#else
    // 回退实现（无优化）
    #define MOB_LIKELY(expr) (expr)
    #define MOB_UNLIKELY(expr) (expr)
#endif

// ==================== 辅助宏 ====================
// 条件语句辅助
#define MOB_IF_LIKELY(expr) if (MOB_LIKELY(expr))
#define MOB_IF_UNLIKELY(expr) if (MOB_UNLIKELY(expr))

// 循环辅助
#define MOB_WHILE_LIKELY(expr) while (MOB_LIKELY(expr))
#define MOB_WHILE_UNLIKELY(expr) while (MOB_UNLIKELY(expr))

// Switch 语句辅助（C++20）
#if defined(MOB_CPP20) || defined(MOB_MSVC_CPP20)
    #define MOB_CASE_LIKELY [[likely]] case
    #define MOB_CASE_UNLIKELY [[unlikely]] case
#else
    #define MOB_CASE_LIKELY case
    #define MOB_CASE_UNLIKELY case
#endif