#include "mob/sync/SyncPoint.hpp"
#include <gtest/gtest.h>
#include <thread>

TEST(SyncTest, SyncPointTest)
{
    mob::sync::SyncPoint syncPoint;
    std::atomic<bool> succ {false};
    auto waitThread = std::thread([&]() {
        succ = syncPoint.wait(std::chrono::seconds(2));
    });
    std::this_thread::sleep_for(std::chrono::seconds(1));
    syncPoint.set();
    waitThread.join();
    EXPECT_TRUE(succ);
}