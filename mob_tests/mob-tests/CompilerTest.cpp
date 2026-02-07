#include "mob/compiler/Likely.hpp"
#include <gtest/gtest.h>
#include <functional>
#include <chrono>
#include <random>
#include <thread>

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

        // warmup
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
            if MOB_LIKELY(value == 1) {
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
            if MOB_UNLIKELY(value == 1) {
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


TEST(CompilerTest, LikelyTest)
{
    // BranchPredictionBenchmark benchmark;
    // auto data = benchmark.generateTestData(10000, 99);

    // constexpr int iteration = 1;
    // constexpr int batchSize = 10000;
    // for (int i = 0; i < iteration; ++i)
    // {
    //     auto likely = benchmark.measure(
    //         batchSize,
    //         &BranchPredictionBenchmark::likelyProcess, &benchmark, data
    //     );

    //     auto baseline = benchmark.measure(
    //         batchSize,
    //         &BranchPredictionBenchmark::baselineProcess, &benchmark, data
    //     );

    //     EXPECT_GE(baseline, likely);
    // }
}

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
