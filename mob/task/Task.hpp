#pragma once

// Main public API for mob::task framework
//
// This framework provides a C++20 task execution and scheduling system
// optimized for minimal latency on time-sensitive tasks.
//
// Key Features:
// - Zero-overhead execution for latency-sensitive tasks (no virtual dispatch)
// - Per-type queues for cache-friendly access
// - Compile-time routing and policy configuration
// - Lock-free concurrent queues
// - Single worker thread for minimal overhead
//
// Usage Example:
//
//   // Define your latency-sensitive task types
//   struct NetworkIOTask {
//       int socketFd;
//       void execute() noexcept { /* handle I/O */ }
//   };
//
//   struct TimerTask {
//       uint64_t deadlineNs;
//       void execute() noexcept { /* handle timer */ }
//   };
//
//   // Create executor with policy and task types
//   // Template parameter order defines priority (first = highest)
//   mob::task::Executor<
//       mob::task::StrictPriorityPolicy,
//       NetworkIOTask,  // Highest priority
//       TimerTask       // Lower priority
//   > executor;
//
//   // Submit tasks
//   NetworkIOTask netTask{42};
//   executor.submitLatencyTask(std::move(netTask));
//
//   executor.submitRegularTask([]() {
//       std::cout << "Regular task\n";
//   });
//
//   // Shutdown when done
//   executor.shutdown();

#include <mob/task/TaskTraits.hpp>
#include <mob/task/TaskQueue.hpp>
#include <mob/task/SchedulingPolicy.hpp>
#include <mob/task/Executor.hpp>

namespace mob::task {

// Re-export main types for convenience
using mob::task::ExecutableTask;
using mob::task::RegularTask;
using mob::task::TaskQueue;
using mob::task::StrictPriorityPolicy;
using mob::task::RoundRobinPolicy;
using mob::task::CustomPriorityPolicy;
using mob::task::ExecutorConfig;
using mob::task::Executor;

} // namespace mob::task
