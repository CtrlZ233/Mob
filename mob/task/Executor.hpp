#pragma once

#include <atomic>
#include <memory>
#include <thread>
#include <tuple>
#include <utility>
#include <mob/sync/SyncPoint.hpp>
#include <mob/task/TaskQueue.hpp>
#include <mob/task/TaskTraits.hpp>
#include <mob/task/SchedulingPolicy.hpp>

namespace mob::task {

// Configuration for executor
struct ExecutorConfig {
    // Spin-wait for latency-sensitive tasks (no sleep)
    bool spinWait = true;

    // Initial capacity for each queue
    std::size_t queueCapacity = 1024;
};

// ============================================================================
// Executor: Single worker thread execution engine
// ============================================================================
// Template parameters:
//   - PolicyTemplate: Scheduling policy template (e.g., StrictPriorityPolicy)
//   - LatencyTaskTypes: User's latency-sensitive task types
//
// Design:
//   - Single worker thread for minimal overhead
//   - Per-type queues for latency-sensitive tasks (cache-friendly)
//   - Separate queue for regular tasks
//   - Zero virtual function overhead
//   - Compile-time routing to correct queue
template<template<typename...> class PolicyTemplate, typename... LatencyTaskTypes>
class Executor {
    static_assert(sizeof...(LatencyTaskTypes) > 0, "At least one latency task type required");
    static_assert((ExecutableTask<LatencyTaskTypes> && ...), "All latency task types must satisfy ExecutableTask concept");

public:
    explicit Executor(const ExecutorConfig& config = ExecutorConfig{})
        : config_(config)
        , latencyQueues_(createQueues(std::make_index_sequence<sizeof...(LatencyTaskTypes)>{}))
        , regularQueue_(config.queueCapacity)
        , policy_()
        , running_(false)
        , shutdownRequested_(false)
        , latencyTaskCount_(0) {
        start();
    }

    ~Executor() {
        shutdown();
    }

    // Disable copy and move
    Executor(const Executor&) = delete;
    Executor& operator=(const Executor&) = delete;
    Executor(Executor&&) = delete;
    Executor& operator=(Executor&&) = delete;

    // Submit latency-sensitive task
    // Compile-time routing to correct queue based on task type
    template<typename TaskType>
    [[nodiscard]] bool submitLatencyTask(TaskType&& task) noexcept {
        static_assert(isOneOf<std::decay_t<TaskType>, LatencyTaskTypes...>,
                      "Task type not registered with executor");
        auto& queue = std::get<TaskQueue<std::decay_t<TaskType>>>(latencyQueues_);
        return queue.tryEnqueue(std::forward<TaskType>(task));
    }

    // Submit regular task
    void submitRegularTask(std::function<void()> func) {
        regularQueue_.enqueue(RegularTask{std::move(func)});
    }

    // Graceful shutdown
    void shutdown() {
        if (!running_.load(std::memory_order_acquire)) {
            return;
        }

        shutdownRequested_.store(true, std::memory_order_release);
        shutdownSync_.set();

        if (workerThread_.joinable()) {
            workerThread_.join();
        }

        running_.store(false, std::memory_order_release);
    }

    // Wait for all tasks to complete
    void waitIdle() {
        while (!allQueuesEmpty()) {
            std::this_thread::yield();
        }
    }

    // Query operations
    [[nodiscard]] bool isRunning() const noexcept {
        return running_.load(std::memory_order_acquire);
    }

    [[nodiscard]] std::size_t regularQueueSize() const noexcept {
        return regularQueue_.size();
    }

    template<typename TaskType>
    [[nodiscard]] std::size_t latencyQueueSize() const noexcept {
        static_assert(isOneOf<TaskType, LatencyTaskTypes...>,
                      "Task type not registered with executor");
        const auto& queue = std::get<TaskQueue<TaskType>>(latencyQueues_);
        return queue.size();
    }

private:
    using Policy = PolicyTemplate<LatencyTaskTypes...>;
    using LatencyQueues = std::tuple<TaskQueue<LatencyTaskTypes>...>;

    // Helper to create queues with initial capacity
    template<std::size_t... Is>
    LatencyQueues createQueues(std::index_sequence<Is...>) {
        return LatencyQueues{TaskQueue<LatencyTaskTypes>(config_.queueCapacity)...};
    }

    // Start worker thread
    void start() {
        running_.store(true, std::memory_order_release);
        workerThread_ = std::thread([this]() { workerLoop(); });
    }

    // Main worker loop
    void workerLoop() {
        while (!shutdownRequested_.load(std::memory_order_acquire)) {
            bool taskExecuted = false;

            // Check if we should process a regular task
            if (shouldProcessRegularTask()) {
                RegularTask task;
                if (regularQueue_.tryDequeue(task)) {
                    task.execute();
                    taskExecuted = true;
                }
            }

            // Check latency-sensitive queues in priority order
            if (!taskExecuted) {
                taskExecuted = checkQueuesInPriorityOrder();
            }

            // If no task was executed and we're in spin mode, yield
            if (!taskExecuted && !config_.spinWait) {
                std::this_thread::yield();
            }
        }

        // Process remaining tasks before shutdown
        drainQueues();
    }

    // Check if we should process a regular task based on policy
    bool shouldProcessRegularTask() noexcept {
        if (regularQueue_.empty()) {
            return false;
        }

        const auto count = latencyTaskCount_.fetch_add(1, std::memory_order_relaxed);
        return (count % Policy::regularTaskInterval) == 0;
    }

    // Check queues in priority order defined by policy
    bool checkQueuesInPriorityOrder() noexcept {
        return checkQueuesImpl(std::make_index_sequence<sizeof...(LatencyTaskTypes)>{});
    }

    // Implementation using fold expression for compile-time iteration
    template<std::size_t... Is>
    bool checkQueuesImpl(std::index_sequence<Is...>) noexcept {
        if constexpr (Policy::strictPriority) {
            // Strict priority: continue current queue while tasks available
            return (tryExecuteFromQueueByIndex<Is>() || ...);
        } else {
            // Round-robin: check each queue once
            bool executed = false;
            ((executed = executed || tryExecuteFromQueueByIndex<Is>()), ...);
            return executed;
        }
    }

    // Try to execute a task from queue at index I
    template<std::size_t I>
    bool tryExecuteFromQueueByIndex() noexcept {
        using TaskType = typeAt<I, LatencyTaskTypes...>;
        auto& queue = std::get<I>(latencyQueues_);

        TaskType task;
        if (queue.tryDequeue(task)) {
            task.execute();
            return true;
        }
        return false;
    }

    // Check if all queues are empty
    bool allQueuesEmpty() const noexcept {
        bool latencyEmpty = allLatencyQueuesEmpty(std::make_index_sequence<sizeof...(LatencyTaskTypes)>{});
        return latencyEmpty && regularQueue_.empty();
    }

    template<std::size_t... Is>
    bool allLatencyQueuesEmpty(std::index_sequence<Is...>) const noexcept {
        return (std::get<Is>(latencyQueues_).empty() && ...);
    }

    // Drain all queues during shutdown
    void drainQueues() noexcept {
        // Drain latency queues
        drainLatencyQueues(std::make_index_sequence<sizeof...(LatencyTaskTypes)>{});

        // Drain regular queue
        RegularTask task;
        while (regularQueue_.tryDequeue(task)) {
            task.execute();
        }
    }

    template<std::size_t... Is>
    void drainLatencyQueues(std::index_sequence<Is...>) noexcept {
        (drainQueueByIndex<Is>(), ...);
    }

    template<std::size_t I>
    void drainQueueByIndex() noexcept {
        using TaskType = typeAt<I, LatencyTaskTypes...>;
        auto& queue = std::get<I>(latencyQueues_);

        TaskType task;
        while (queue.tryDequeue(task)) {
            task.execute();
        }
    }

    // Member data
    ExecutorConfig config_;
    LatencyQueues latencyQueues_;
    TaskQueue<RegularTask> regularQueue_;
    Policy policy_;

    std::thread workerThread_;
    std::atomic<bool> running_;
    std::atomic<bool> shutdownRequested_;
    std::atomic<std::size_t> latencyTaskCount_;
    mob::sync::SyncPoint shutdownSync_;
};

} // namespace mob::task
