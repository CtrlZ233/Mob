#include <mob/task/Task.hpp>
#include <iostream>
#include <chrono>
#include <thread>

// Define latency-sensitive task types
struct NetworkIOTask {
    int socketFd;

    void execute() noexcept {
        // Simulate network I/O processing
        std::cout << "Processing network I/O on socket " << socketFd << "\n";
    }
};

struct TimerTask {
    uint64_t deadlineNs;

    void execute() noexcept {
        // Simulate timer handling
        std::cout << "Timer expired at " << deadlineNs << " ns\n";
    }
};

struct SignalTask {
    int signalNum;

    void execute() noexcept {
        // Simulate signal handling
        std::cout << "Handling signal " << signalNum << "\n";
    }
};

int main() {
    std::cout << "=== Mob Task Framework Example ===\n\n";

    // Create executor with strict priority policy
    // Priority order: NetworkIOTask > TimerTask > SignalTask
    mob::task::ExecutorConfig config;
    config.queueCapacity = 1024;
    config.spinWait = true;  // Spin-wait for minimal latency

    mob::task::Executor<
        mob::task::StrictPriorityPolicy,
        NetworkIOTask,  // Highest priority
        TimerTask,      // Medium priority
        SignalTask      // Lowest priority
    > executor(config);

    std::cout << "Executor started with 3 task types\n\n";

    // Submit latency-sensitive tasks
    std::cout << "Submitting latency-sensitive tasks...\n";

    for (int i = 0; i < 5; ++i) {
        executor.submitLatencyTask(NetworkIOTask{100 + i});
        executor.submitLatencyTask(TimerTask{1000000 + i * 1000});
        executor.submitLatencyTask(SignalTask{i + 1});
    }

    // Submit regular tasks
    std::cout << "Submitting regular tasks...\n";

    executor.submitRegularTask([]() {
        std::cout << "Regular task 1: Performing background work\n";
    });

    executor.submitRegularTask([]() {
        std::cout << "Regular task 2: Logging statistics\n";
    });

    // Wait for all tasks to complete
    std::cout << "\nWaiting for tasks to complete...\n";
    executor.waitIdle();

    std::cout << "\nAll tasks completed!\n";

    // Shutdown executor
    executor.shutdown();
    std::cout << "Executor shutdown complete\n";

    return 0;
}
