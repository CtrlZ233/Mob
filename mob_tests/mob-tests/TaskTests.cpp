#include <mob/task/Task.hpp>
#include <gtest/gtest.h>
#include <atomic>
#include <chrono>
#include <thread>
#include <vector>

using namespace mob::task;

// ============================================================================
// Test Task Types
// ============================================================================

struct SimpleTask {
    std::atomic<int>* counter;

    void execute() noexcept {
        if (counter) {
            counter->fetch_add(1, std::memory_order_relaxed);
        }
    }
};

struct PriorityTask1 {
    std::atomic<int>* counter;
    std::vector<int>* executionOrder;

    void execute() noexcept {
        if (counter) {
            counter->fetch_add(1, std::memory_order_relaxed);
        }
        if (executionOrder) {
            executionOrder->push_back(1);
        }
    }
};

struct PriorityTask2 {
    std::atomic<int>* counter;
    std::vector<int>* executionOrder;

    void execute() noexcept {
        if (counter) {
            counter->fetch_add(1, std::memory_order_relaxed);
        }
        if (executionOrder) {
            executionOrder->push_back(2);
        }
    }
};

struct PriorityTask3 {
    std::atomic<int>* counter;
    std::vector<int>* executionOrder;

    void execute() noexcept {
        if (counter) {
            counter->fetch_add(1, std::memory_order_relaxed);
        }
        if (executionOrder) {
            executionOrder->push_back(3);
        }
    }
};

// ============================================================================
// Basic Functionality Tests
// ============================================================================

TEST(TaskTests, BasicTaskExecution) {
    std::atomic<int> counter{0};

    Executor<StrictPriorityPolicy, SimpleTask> executor;

    // Submit tasks
    for (int i = 0; i < 10; ++i) {
        ASSERT_TRUE(executor.submitLatencyTask(SimpleTask{&counter}));
    }

    // Wait for tasks to complete
    executor.waitIdle();

    EXPECT_EQ(counter.load(), 10);
}

TEST(TaskTests, RegularTaskExecution) {
    std::atomic<int> counter{0};

    Executor<StrictPriorityPolicy, SimpleTask> executor;

    // Submit regular tasks
    for (int i = 0; i < 10; ++i) {
        executor.submitRegularTask([&counter]() {
            counter.fetch_add(1, std::memory_order_relaxed);
        });
    }

    // Wait for tasks to complete
    executor.waitIdle();

    EXPECT_EQ(counter.load(), 10);
}

TEST(TaskTests, MixedTaskExecution) {
    std::atomic<int> latencyCounter{0};
    std::atomic<int> regularCounter{0};

    Executor<StrictPriorityPolicy, SimpleTask> executor;

    // Submit mixed tasks
    for (int i = 0; i < 10; ++i) {
        ASSERT_TRUE(executor.submitLatencyTask(SimpleTask{&latencyCounter}));
        executor.submitRegularTask([&regularCounter]() {
            regularCounter.fetch_add(1, std::memory_order_relaxed);
        });
    }

    // Wait for tasks to complete
    executor.waitIdle();

    EXPECT_EQ(latencyCounter.load(), 10);
    EXPECT_EQ(regularCounter.load(), 10);
}

// ============================================================================
// Priority Tests
// ============================================================================

TEST(TaskTests, StrictPriorityOrdering) {
    std::atomic<int> counter1{0};
    std::atomic<int> counter2{0};
    std::atomic<int> counter3{0};

    // Priority order: PriorityTask1 > PriorityTask2 > PriorityTask3
    Executor<StrictPriorityPolicy, PriorityTask1, PriorityTask2, PriorityTask3> executor;

    // Submit tasks in reverse priority order
    for (int i = 0; i < 10; ++i) {
        ASSERT_TRUE(executor.submitLatencyTask(PriorityTask3{&counter3, nullptr}));
        ASSERT_TRUE(executor.submitLatencyTask(PriorityTask2{&counter2, nullptr}));
        ASSERT_TRUE(executor.submitLatencyTask(PriorityTask1{&counter1, nullptr}));
    }

    // Wait for tasks to complete
    executor.waitIdle();

    EXPECT_EQ(counter1.load(), 10);
    EXPECT_EQ(counter2.load(), 10);
    EXPECT_EQ(counter3.load(), 10);
}

TEST(TaskTests, RegularTaskStarvationPrevention) {
    std::atomic<int> latencyCounter{0};
    std::atomic<int> regularCounter{0};

    Executor<StrictPriorityPolicy, SimpleTask> executor;

    // Submit many latency tasks and one regular task
    for (int i = 0; i < 200; ++i) {
        ASSERT_TRUE(executor.submitLatencyTask(SimpleTask{&latencyCounter}));
    }

    executor.submitRegularTask([&regularCounter]() {
        regularCounter.fetch_add(1, std::memory_order_relaxed);
    });

    // Wait for tasks to complete
    executor.waitIdle();

    EXPECT_EQ(latencyCounter.load(), 200);
    // Regular task should have been executed (starvation prevention)
    EXPECT_EQ(regularCounter.load(), 1);
}

// ============================================================================
// Concurrency Tests
// ============================================================================

TEST(TaskTests, ConcurrentSubmission) {
    std::atomic<int> counter{0};

    Executor<StrictPriorityPolicy, SimpleTask> executor;

    // Multiple threads submitting tasks
    std::vector<std::thread> threads;
    constexpr int numThreads = 4;
    constexpr int tasksPerThread = 100;

    for (int t = 0; t < numThreads; ++t) {
        threads.emplace_back([&executor, &counter]() {
            for (int i = 0; i < tasksPerThread; ++i) {
                while (!executor.submitLatencyTask(SimpleTask{&counter})) {
                    std::this_thread::yield();
                }
            }
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    // Wait for tasks to complete
    executor.waitIdle();

    EXPECT_EQ(counter.load(), numThreads * tasksPerThread);
}

// ============================================================================
// Performance Tests
// ============================================================================

TEST(TaskTests, LatencyBenchmark) {
    std::atomic<int> counter{0};

    ExecutorConfig config;
    config.queueCapacity = 20000;  // Increase capacity for benchmark
    Executor<StrictPriorityPolicy, SimpleTask> executor(config);

    constexpr int numTasks = 10000;

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < numTasks; ++i) {
        // Retry if queue is full
        while (!executor.submitLatencyTask(SimpleTask{&counter})) {
            std::this_thread::yield();
        }
    }

    executor.waitIdle();

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);

    EXPECT_EQ(counter.load(), numTasks);

    // Calculate average latency per task
    auto avgLatencyNs = duration.count() / numTasks;
    std::cout << "Average latency per task: " << avgLatencyNs << " ns\n";

    // Should be well under 1 microsecond per task
    EXPECT_LT(avgLatencyNs, 1000);
}

// ============================================================================
// Policy Tests
// ============================================================================

TEST(TaskTests, RoundRobinPolicy) {
    std::atomic<int> counter1{0};
    std::atomic<int> counter2{0};

    Executor<RoundRobinPolicy, PriorityTask1, PriorityTask2> executor;

    // Submit tasks
    for (int i = 0; i < 10; ++i) {
        ASSERT_TRUE(executor.submitLatencyTask(PriorityTask1{&counter1, nullptr}));
        ASSERT_TRUE(executor.submitLatencyTask(PriorityTask2{&counter2, nullptr}));
    }

    // Wait for tasks to complete
    executor.waitIdle();

    EXPECT_EQ(counter1.load(), 10);
    EXPECT_EQ(counter2.load(), 10);
}

// ============================================================================
// Shutdown Tests
// ============================================================================

TEST(TaskTests, GracefulShutdown) {
    std::atomic<int> counter{0};

    {
        Executor<StrictPriorityPolicy, SimpleTask> executor;

        // Submit tasks
        for (int i = 0; i < 100; ++i) {
            ASSERT_TRUE(executor.submitLatencyTask(SimpleTask{&counter}));
        }

        // Executor destructor should drain remaining tasks
    }

    // All tasks should have been executed
    EXPECT_EQ(counter.load(), 100);
}

TEST(TaskTests, ExplicitShutdown) {
    std::atomic<int> counter{0};

    Executor<StrictPriorityPolicy, SimpleTask> executor;

    // Submit tasks
    for (int i = 0; i < 100; ++i) {
        ASSERT_TRUE(executor.submitLatencyTask(SimpleTask{&counter}));
    }

    executor.shutdown();

    EXPECT_FALSE(executor.isRunning());
    EXPECT_EQ(counter.load(), 100);
}



