#pragma once

#include <chrono>
#include <condition_variable>
#include <mutex>

namespace mob
{

namespace sync
{

class SyncPoint
{
public:
    SyncPoint() = default;

    bool wait(std::chrono::nanoseconds ns);

    bool wait();

    void set();

private:
    std::mutex mutex_;
    std::condition_variable cv_;
    bool ready_ {false};
};

}

}

