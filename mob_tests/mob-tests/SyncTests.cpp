#include "mob/sync/SyncPoint.hpp"
#include <gtest/gtest.h>
#include <thread>
#include <chrono>
#include <atomic>

using namespace mob::sync;
using namespace std::chrono_literals;

TEST(SyncTest, BasicSetAndWait)
{
    SyncPoint sp;

    std::thread t([&sp]() {
        std::this_thread::sleep_for(50ms);
        sp.set();
    });

    bool result = sp.wait();
    EXPECT_TRUE(result);

    t.join();
}

TEST(SyncTest, WaitWithTimeoutSuccess)
{
    SyncPoint sp;

    std::thread t([&sp]() {
        std::this_thread::sleep_for(50ms);
        sp.set();
    });

    bool result = sp.wait(500ms);
    EXPECT_TRUE(result);

    t.join();
}

TEST(SyncTest, WaitWithTimeoutFailure)
{
    SyncPoint sp;

    auto start = std::chrono::steady_clock::now();
    bool result = sp.wait(100ms);
    auto end = std::chrono::steady_clock::now();

    EXPECT_FALSE(result);
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    EXPECT_GE(duration.count(), 100);
}

TEST(SyncTest, SetBeforeWait)
{
    SyncPoint sp;
    sp.set();

    auto start = std::chrono::steady_clock::now();
    bool result = sp.wait();
    auto end = std::chrono::steady_clock::now();

    EXPECT_TRUE(result);
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    EXPECT_LT(duration.count(), 50);
}

TEST(SyncTest, WaitBeforeSet)
{
    SyncPoint sp;
    std::atomic<bool> waitStarted{false};
    std::atomic<bool> waitCompleted{false};

    std::thread t([&sp, &waitStarted, &waitCompleted]() {
        waitStarted = true;
        bool result = sp.wait();
        waitCompleted = true;
        EXPECT_TRUE(result);
    });

    while (!waitStarted) {
        std::this_thread::sleep_for(1ms);
    }

    std::this_thread::sleep_for(50ms);
    EXPECT_FALSE(waitCompleted);

    sp.set();

    t.join();
    EXPECT_TRUE(waitCompleted);
}

TEST(SyncTest, RepeatedSetWait)
{
    for (int i = 0; i < 5; ++i) {
        SyncPoint sp;

        std::thread t([&sp]() {
            std::this_thread::sleep_for(10ms);
            sp.set();
        });

        bool result = sp.wait();
        EXPECT_TRUE(result);

        t.join();
    }
}