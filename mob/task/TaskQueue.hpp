#pragma once

#include <atomic>
#include <cstddef>
#include <concurrentqueue.h>

namespace mob::task {

// Lock-free task queue wrapper using moodycamel::ConcurrentQueue
template<typename T>
class TaskQueue {
public:
    explicit TaskQueue(std::size_t initialCapacity = 1024)
        : queue_(initialCapacity), queueSize_(0) {}

    // Move constructor and assignment
    TaskQueue(TaskQueue&& other) noexcept
        : queue_(std::move(other.queue_))
        , queueSize_(other.queueSize_.load(std::memory_order_relaxed)) {}

    TaskQueue& operator=(TaskQueue&& other) noexcept {
        if (this != &other) {
            queue_ = std::move(other.queue_);
            queueSize_.store(other.queueSize_.load(std::memory_order_relaxed), std::memory_order_relaxed);
        }
        return *this;
    }

    // Delete copy constructor and assignment
    TaskQueue(const TaskQueue&) = delete;
    TaskQueue& operator=(const TaskQueue&) = delete;

    // Non-blocking enqueue for latency-sensitive tasks
    [[nodiscard]] bool tryEnqueue(T&& task) noexcept {
        if (queue_.try_enqueue(std::forward<T>(task))) {
            queueSize_.fetch_add(1, std::memory_order_relaxed);
            return true;
        }
        return false;
    }

    // Blocking enqueue for regular tasks (can allocate if needed)
    void enqueue(T&& task) {
        queue_.enqueue(std::forward<T>(task));
        queueSize_.fetch_add(1, std::memory_order_relaxed);
    }

    // Non-blocking dequeue
    [[nodiscard]] bool tryDequeue(T& task) noexcept {
        if (queue_.try_dequeue(task)) {
            queueSize_.fetch_sub(1, std::memory_order_relaxed);
            return true;
        }
        return false;
    }

    // Bulk dequeue for batch processing
    template<typename It>
    std::size_t tryDequeueBulk(It itemsBegin, std::size_t maxItems) noexcept {
        std::size_t count = queue_.try_dequeue_bulk(itemsBegin, maxItems);
        if (count > 0) {
            queueSize_.fetch_sub(count, std::memory_order_relaxed);
        }
        return count;
    }

    // Query operations
    [[nodiscard]] bool empty() const noexcept {
        return queueSize_.load(std::memory_order_relaxed) == 0;
    }

    [[nodiscard]] std::size_t size() const noexcept {
        return queueSize_.load(std::memory_order_relaxed);
    }

private:
    moodycamel::ConcurrentQueue<T> queue_;
    std::atomic<std::size_t> queueSize_;
};

} // namespace mob::task
