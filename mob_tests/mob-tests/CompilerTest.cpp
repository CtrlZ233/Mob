#include "mob/compiler/Likely.hpp"
#include <gtest/gtest.h>
#include <functional>
#include <chrono>
#include <random>
#include <thread>
#include <vector>
#include <numeric>
#include <iomanip>
#include <iostream>

// ============================================================================
// Test Utilities
// ============================================================================

class BranchPredictionBenchmark {
private:
    std::mt19937 gen;
    std::uniform_int_distribution<int> dist;

public:
    BranchPredictionBenchmark() : gen(42), dist(0, 100) {}

    template<typename Func, typename ...Args>
    static int64_t measure(int iterations, Func &&func, Args&&... args)
    {
        using namespace std::chrono;

        // Warmup
        for (int i = 0; i < 10; ++i)
        {
            std::invoke(func, std::forward<Args>(args)...);
        }

        auto start = high_resolution_clock::now();
        volatile int result = 0;
        for (int i = 0; i < iterations; ++i)
        {
            result += std::invoke(func, std::forward<Args>(args)...);
        }
        auto end = high_resolution_clock::now();
        auto duration = duration_cast<nanoseconds>(end - start);

        return duration.count();
    }

    std::vector<int> generateTestData(size_t size, int hotPathProbability) {
        std::vector<int> data(size);
        for (auto& val : data)
        {
            val = dist(gen) < hotPathProbability ? 1 : 0;
        }
        return data;
    }

    int baselineProcess(const std::vector<int>& data) {
        int result = 0;
        for (int value : data) {
            if (value == 1) {
                result += expensiveOperation(value);
            } else {
                result += cheapOperation(value);
            }
        }
        return result;
    }

    int likelyProcess(const std::vector<int>& data) {
        int result = 0;
        for (int value : data) {
            MOB_IF_LIKELY(value == 1) {
                result += expensiveOperation(value);
            } else {
                result += cheapOperation(value);
            }
        }
        return result;
    }

    int unlikelyProcess(const std::vector<int>& data) {
        int result = 0;
        for (int value : data) {
            MOB_IF_UNLIKELY(value == 1) {
                result += expensiveOperation(value);
            } else {
                result += cheapOperation(value);
            }
        }
        return result;
    }

private:
    int expensiveOperation(int value) {
        int sum = 0;
        for (int i = 0; i < 100; ++i) {
            sum += value * i;
        }
        return sum;
    }

    int cheapOperation(int value) {
        return value + 1;
    }
};

// ============================================================================
// Basic Functionality Tests
// ============================================================================

TEST(CompilerTest, MacroExpansion)
{
    int x = 5;

    bool result1 = MOB_LIKELY(x > 0);
    EXPECT_TRUE(result1);

    bool result2 = MOB_UNLIKELY(x < 0);
    EXPECT_FALSE(result2);
}

TEST(CompilerTest, LikelyUnlikelyBasic)
{
    int counter = 0;

    for (int i = 0; i < 100; ++i) {
        if (MOB_LIKELY(i < 95)) {
            counter++;
        }
    }

    EXPECT_EQ(counter, 95);

    counter = 0;
    for (int i = 0; i < 100; ++i) {
        if (MOB_UNLIKELY(i >= 95)) {
            counter++;
        }
    }

    EXPECT_EQ(counter, 5);
}

TEST(CompilerTest, IfLikelyMacro)
{
    int value = 10;
    bool executed = false;

    MOB_IF_LIKELY(value > 5) {
        executed = true;
    }

    EXPECT_TRUE(executed);
}

TEST(CompilerTest, IfUnlikelyMacro)
{
    int value = 10;
    bool executed = false;

    MOB_IF_UNLIKELY(value < 5) {
        executed = true;
    }

    EXPECT_FALSE(executed);
}

TEST(CompilerTest, WhileLikelyMacro)
{
    int counter = 0;
    int i = 0;

    MOB_WHILE_LIKELY(i < 10) {
        counter++;
        i++;
    }

    EXPECT_EQ(counter, 10);
}

// ============================================================================
// C++20 Statement-level Attribute Tests
// ============================================================================

TEST(CompilerTest, StatementLevelAttributes)
{
    int value = 10;
    int result = 0;

    // Test MOB_LIKELY_BRANCH
    if (value > 5) MOB_LIKELY_BRANCH {
        result = 1;
    } else MOB_UNLIKELY_BRANCH {
        result = 2;
    }

    EXPECT_EQ(result, 1);

    // Test with unlikely condition
    if (value < 5) MOB_UNLIKELY_BRANCH {
        result = 3;
    } else MOB_LIKELY_BRANCH {
        result = 4;
    }

    EXPECT_EQ(result, 4);
}

TEST(CompilerTest, SwitchCaseLikely)
{
    int result = 0;

    for (int i = 0; i < 100; ++i) {
        switch (i % 10) {
            MOB_CASE_LIKELY 0:
                result += 10;
                break;
            MOB_CASE_UNLIKELY 9:
                result += 1;
                break;
            default:
                result += 5;
                break;
        }
    }

    // 10 cases of 0 (10*10=100), 10 cases of 9 (10*1=10), 80 cases of others (80*5=400)
    EXPECT_EQ(result, 510);
}

// ============================================================================
// Performance Tests
// ============================================================================

TEST(CompilerTest, HotPathPerformance)
{
    BranchPredictionBenchmark benchmark;

    // Generate data where condition is true 95% of the time (hot path)
    auto data = benchmark.generateTestData(10000, 95);

    constexpr int iterations = 100;

    // Measure baseline
    auto baselineTime = benchmark.measure(
        iterations,
        &BranchPredictionBenchmark::baselineProcess, &benchmark, data
    );

    // Measure with LIKELY hint
    auto likelyTime = benchmark.measure(
        iterations,
        &BranchPredictionBenchmark::likelyProcess, &benchmark, data
    );

    // Measure with UNLIKELY hint (wrong hint)
    auto unlikelyTime = benchmark.measure(
        iterations,
        &BranchPredictionBenchmark::unlikelyProcess, &benchmark, data
    );

    std::cout << "\n=== Hot Path Performance (95% true) ===\n";
    std::cout << "Baseline:  " << baselineTime << " ns\n";
    std::cout << "LIKELY:    " << likelyTime << " ns";
    if (likelyTime < baselineTime) {
        std::cout << " (faster by " << (baselineTime - likelyTime) << " ns, "
                  << std::fixed << std::setprecision(1)
                  << (100.0 * (baselineTime - likelyTime) / baselineTime) << "%)";
    }
    std::cout << "\n";
    std::cout << "UNLIKELY:  " << unlikelyTime << " ns";
    if (unlikelyTime > baselineTime) {
        std::cout << " (slower by " << (unlikelyTime - baselineTime) << " ns, "
                  << std::fixed << std::setprecision(1)
                  << (100.0 * (unlikelyTime - baselineTime) / baselineTime) << "%)";
    }
    std::cout << "\n\n";

    // LIKELY should be at least as fast as baseline (or very close)
    // UNLIKELY should be slower than LIKELY for hot path
    EXPECT_LE(likelyTime, baselineTime * 1.1);  // Allow 10% variance
    EXPECT_GE(unlikelyTime, likelyTime * 0.9);  // UNLIKELY should not be much faster
}

TEST(CompilerTest, ColdPathPerformance)
{
    BranchPredictionBenchmark benchmark;

    // Generate data where condition is true only 5% of the time (cold path)
    auto data = benchmark.generateTestData(10000, 5);

    constexpr int iterations = 100;

    // Measure baseline
    auto baselineTime = benchmark.measure(
        iterations,
        &BranchPredictionBenchmark::baselineProcess, &benchmark, data
    );

    // Measure with LIKELY hint (wrong hint)
    auto likelyTime = benchmark.measure(
        iterations,
        &BranchPredictionBenchmark::likelyProcess, &benchmark, data
    );

    // Measure with UNLIKELY hint (correct hint)
    auto unlikelyTime = benchmark.measure(
        iterations,
        &BranchPredictionBenchmark::unlikelyProcess, &benchmark, data
    );

    std::cout << "\n=== Cold Path Performance (5% true) ===\n";
    std::cout << "Baseline:  " << baselineTime << " ns\n";
    std::cout << "LIKELY:    " << likelyTime << " ns";
    if (likelyTime > baselineTime) {
        std::cout << " (slower by " << (likelyTime - baselineTime) << " ns, "
                  << std::fixed << std::setprecision(1)
                  << (100.0 * (likelyTime - baselineTime) / baselineTime) << "%)";
    }
    std::cout << "\n";
    std::cout << "UNLIKELY:  " << unlikelyTime << " ns";
    if (unlikelyTime < baselineTime) {
        std::cout << " (faster by " << (baselineTime - unlikelyTime) << " ns, "
                  << std::fixed << std::setprecision(1)
                  << (100.0 * (baselineTime - unlikelyTime) / baselineTime) << "%)";
    }
    std::cout << "\n\n";

    // UNLIKELY should be at least as fast as baseline (or very close)
    // LIKELY should be slower than UNLIKELY for cold path
    EXPECT_LE(unlikelyTime, baselineTime * 1.1);  // Allow 10% variance
    EXPECT_GE(likelyTime, unlikelyTime * 0.9);    // LIKELY should not be much faster
}

// ============================================================================
// Real-world Usage Pattern Tests
// ============================================================================

TEST(CompilerTest, ErrorHandlingPattern)
{
    auto processWithErrorCheck = [](int value) -> int {
        // Error handling - unlikely path
        MOB_IF_UNLIKELY(value < 0) {
            return -1;
        }

        // Normal processing - likely path
        return value * 2;
    };

    EXPECT_EQ(processWithErrorCheck(10), 20);
    EXPECT_EQ(processWithErrorCheck(-5), -1);
}

TEST(CompilerTest, LoopContinuationPattern)
{
    std::vector<int> data(1000);
    std::iota(data.begin(), data.end(), 0);

    int sum = 0;
    for (size_t i = 0; i < data.size(); ) {
        // Loop continuation is likely
        sum += data[i];
        i++;

        MOB_IF_LIKELY(i < data.size()) {
            continue;
        }
        break;
    }

    EXPECT_EQ(sum, 499500);  // Sum of 0..999
}

TEST(CompilerTest, PointerNullCheckPattern)
{
    int value = 42;
    int* ptr = &value;

    // Null check - unlikely in normal operation
    MOB_IF_UNLIKELY(ptr == nullptr) {
        FAIL() << "Pointer should not be null";
    }

    EXPECT_EQ(*ptr, 42);
}

TEST(CompilerTest, NestedBranchHints)
{
    int result = 0;

    for (int i = 0; i < 100; ++i) {
        // Outer loop condition - likely
        MOB_IF_LIKELY(i < 95) {
            // Inner condition - also likely within this branch
            MOB_IF_LIKELY(i % 2 == 0) {
                result += 2;
            } else {
                result += 1;
            }
        } else {
            // Rare case
            result += 10;
        }
    }

    // 95 iterations: 48*2 + 47*1 = 143
    // 5 iterations: 5*10 = 50
    // Total: 193
    EXPECT_EQ(result, 193);
}

// ============================================================================
// Compiler Macro Detection Tests
// ============================================================================

TEST(CompilerTest, MacroDefinitions)
{
    // Verify that appropriate macros are defined
#if defined(MOB_COMPILER_GCC_LIKE)
    EXPECT_TRUE(true) << "GCC-like compiler detected";
#elif defined(MOB_COMPILER_MSVC)
    EXPECT_TRUE(true) << "MSVC compiler detected";
#else
    EXPECT_TRUE(true) << "Unknown compiler";
#endif

#if defined(MOB_CPP20)
    EXPECT_TRUE(true) << "C++20 detected";
#elif defined(MOB_CPP17)
    EXPECT_TRUE(true) << "C++17 detected";
#else
    EXPECT_TRUE(true) << "C++14 or earlier";
#endif
}
