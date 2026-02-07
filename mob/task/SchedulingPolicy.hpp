#pragma once

#include <cstddef>
#include <tuple>

namespace mob::task {

// ============================================================================
// Scheduling Policy Interface
// ============================================================================
//
// Policies are template classes that configure the executor's scheduling behavior.
// All configuration is constexpr for zero runtime overhead.
//
// Required interface:
//   - static constexpr int regularTaskInterval: Process one regular task every N latency tasks
//   - static constexpr bool strictPriority: Continue current queue while tasks available
//   - using PriorityOrder: std::tuple defining queue checking order
//
// Simple policies just change configuration parameters.
// Advanced policies can redefine PriorityOrder to change priority.
// Expert policies can use TaskTypes for complex compile-time logic.

// ============================================================================
// StrictPriorityPolicy (Default)
// ============================================================================
// Processes latency-sensitive tasks in template parameter order (first = highest priority).
// Continues processing from a queue while tasks are available (strict priority).
// Processes one regular task every 100 latency-sensitive tasks.
template<typename... TaskTypes>
struct StrictPriorityPolicy {
    // Process one regular task every 100 latency-sensitive tasks
    static constexpr int regularTaskInterval = 100;

    // Strict priority: continue current queue while tasks available
    static constexpr bool strictPriority = true;

    // Use default priority order (template parameter order)
    using PriorityOrder = std::tuple<TaskTypes...>;
};

// ============================================================================
// RoundRobinPolicy
// ============================================================================
// Checks each queue once per cycle (round-robin).
// Processes one regular task every 100 latency-sensitive tasks.
template<typename... TaskTypes>
struct RoundRobinPolicy {
    // Process one regular task every 100 latency-sensitive tasks
    static constexpr int regularTaskInterval = 100;

    // Round-robin: check each queue once per cycle
    static constexpr bool strictPriority = false;

    // Use default priority order (template parameter order)
    using PriorityOrder = std::tuple<TaskTypes...>;
};

// ============================================================================
// CustomPriorityPolicy (Example)
// ============================================================================
// Example showing how to redefine priority order.
// This policy reverses the default priority order.
template<typename... TaskTypes>
struct CustomPriorityPolicy {
    static constexpr int regularTaskInterval = 100;
    static constexpr bool strictPriority = true;

    // Custom priority order - can be any permutation of TaskTypes
    // This example just uses the default order, but users can reorder as needed
    using PriorityOrder = std::tuple<TaskTypes...>;
};

} // namespace mob::task
