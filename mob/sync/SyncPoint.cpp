#include "SyncPoint.hpp"

namespace mob
{

namespace sync
{

bool SyncPoint::wait(std::chrono::nanoseconds ns)
{
    std::unique_lock<std::mutex> lock(mutex_);
    if (!cv_.wait_for(lock, ns, [this]() { return ready_; }))
    {
        return false;
    }
    ready_ = false;
    return true;
}

bool SyncPoint::wait()
{
    std::unique_lock<std::mutex> lock(mutex_);
    cv_.wait(lock, [this](){ return ready_; });
    ready_ = false;
    return true;
}

void SyncPoint::set()
{
    ready_ = true;
    cv_.notify_all();
}

}

}