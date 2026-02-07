# Mob Task Framework

A C++20 task execution and scheduling framework optimized for minimal latency on time-sensitive tasks.

## Features

- **Zero-overhead execution** for latency-sensitive tasks (no virtual dispatch, no heap allocation)
- **Per-type queues** for cache-friendly access and zero memory waste
- **Compile-time routing** and policy configuration
- **Lock-free concurrent queues** using moodycamel::ConcurrentQueue
- **Single worker thread** for minimal context switching overhead
- **Flexible scheduling policies** with compile-time configuration
- **Type-safe** task submission enforced by the compiler

## Performance

- **Average latency**: ~73 nanoseconds per task (measured on test system)
- **Zero virtual function overhead**: Direct function calls only
- **Cache-friendly**: Same-type tasks grouped together in dedicated queues
- **Lock-free**: Non-blocking enqueue/dequeue operations

## Quick Start

### 1. Define Your Task Types

```cpp
#include <mob/task/Task.hpp>

// Define latency-sensitive task types
struct NetworkIOTask {
    int socketFd;
    void* buffer;
    size_t size;

    void execute() noexcept {
        // Handle network I/O with minimal overhead
    }
};

struct TimerTask {
    uint64_t deadlineNs;
    void (*callback)(void*) noexcept;
    void* userData;

    void execute() noexcept {
        callback(userData);
    }
};
```

### 2. Create an Executor

```cpp
// Create executor with policy and task types
// Template parameter order defines priority (first = highest)
mob::task::Executor<
    mob::task::StrictPriorityPolicy,
    NetworkIOTask,  // Highest priority
    TimerTask       // Lower priority
> executor;
```

### 3. Submit Tasks

```cpp
// Submit latency-sensitive tasks (compile-time routing)
NetworkIOTask netTask{42, buffer, 1024};
executor.submitLatencyTask(std::move(netTask));

TimerTask timerTask{1000000, myCallback, userData};
executor.submitLatencyTask(std::move(timerTask));

// Submit regular tasks (can use lambda/std::function)
executor.submitRegularTask([]() {
    std::cout << "Regular task executed\n";
});
```

### 4. Shutdown

```cpp
// Graceful shutdown (processes remaining tasks)
executor.shutdown();
```

## Architecture

### Task Types

**Latency-Sensitive Tasks**:
- Must have a `void execute()` or `void execute() noexcept` method
- Each type gets its own dedicated queue
- Zero overhead: no virtual functions, no heap allocation
- Compile-time routing to correct queue

**Regular Tasks**:
- Use `std::function<void()>` for flexibility
- Separate queue from latency-sensitive tasks
- Suitable for background work, logging, etc.

### Priority System

Priority is defined by template parameter order:
```cpp
Executor<Policy, Task1, Task2, Task3> executor;
// Task1 has highest priority (index 0)
// Task2 has medium priority (index 1)
// Task3 has lowest priority (index 2)
```

### Scheduling Policies

#### StrictPriorityPolicy (Default)

```cpp
template<typename... TaskTypes>
struct StrictPriorityPolicy {
    static constexpr int regularTaskInterval = 100;
    static constexpr bool strictPriority = true;
    using PriorityOrder = std::tuple<TaskTypes...>;
};
```

- Processes latency-sensitive tasks in template parameter order
- Continues processing from a queue while tasks are available
- Processes one regular task every 100 latency-sensitive tasks

#### RoundRobinPolicy

```cpp
template<typename... TaskTypes>
struct RoundRobinPolicy {
    static constexpr int regularTaskInterval = 100;
    static constexpr bool strictPriority = false;
    using PriorityOrder = std::tuple<TaskTypes...>;
};
```

- Checks each queue once per cycle (round-robin)
- More fair distribution among task types

#### Custom Policies

Create your own policy by defining:
- `regularTaskInterval`: How often to process regular tasks
- `strictPriority`: Continue current queue vs. round-robin
- `PriorityOrder`: Custom priority ordering (optional)

```cpp
template<typename... TaskTypes>
struct MyCustomPolicy {
    static constexpr int regularTaskInterval = 50;
    static constexpr bool strictPriority = true;

    // Reorder priorities if needed
    using PriorityOrder = std::tuple<TaskTypes...>;
};

Executor<MyCustomPolicy, Task1, Task2> executor;
```

## Configuration

```cpp
mob::task::ExecutorConfig config;
config.spinWait = true;          // Spin-wait for minimal latency (default)
config.queueCapacity = 1024;     // Initial capacity for each queue (default)

Executor<Policy, TaskTypes...> executor(config);
```

## API Reference

### Executor

```cpp
template<template<typename...> class PolicyTemplate, typename... LatencyTaskTypes>
class Executor;
```

**Methods**:
- `submitLatencyTask<TaskType>(task)`: Submit latency-sensitive task (non-blocking)
- `submitRegularTask(func)`: Submit regular task (blocking)
- `shutdown()`: Graceful shutdown
- `waitIdle()`: Wait for all tasks to complete
- `isRunning()`: Check if executor is running
- `regularQueueSize()`: Get regular queue size
- `latencyQueueSize<TaskType>()`: Get latency queue size for specific type

### TaskQueue

```cpp
template<typename T>
class TaskQueue;
```

**Methods**:
- `tryEnqueue(task)`: Non-blocking enqueue
- `enqueue(task)`: Blocking enqueue
- `tryDequeue(task)`: Non-blocking dequeue
- `tryDequeueBulk(begin, maxItems)`: Bulk dequeue
- `empty()`: Check if queue is empty
- `size()`: Get queue size

## Design Principles

1. **Latency First**: Minimize overhead for time-sensitive tasks
2. **Type Safety**: Compiler-enforced correctness
3. **Zero Cost Abstractions**: No runtime overhead for compile-time decisions
4. **Cache Friendly**: Per-type queues for better locality
5. **Lock-Free**: Non-blocking operations where possible

## Implementation Details

- **Single worker thread**: Eliminates context switching overhead
- **Per-type queues**: Each task type has dedicated `TaskQueue<TaskType>`
- **Compile-time routing**: `std::get<TaskQueue<TaskType>>` for zero overhead
- **Template-based policies**: All configuration is `constexpr`, fully inlined
- **Lock-free queues**: Using moodycamel::ConcurrentQueue

## Testing

Run the test suite:
```bash
cd mob_tests
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
cmake --build build/
./build/mob-tests/TaskTests
```

## Examples

See `examples/task_example.cpp` for a complete working example.

## Requirements

- C++20 compiler
- CMake 3.12+
- moodycamel::ConcurrentQueue (included in third-party/)

## License

Part of the Mob library.
