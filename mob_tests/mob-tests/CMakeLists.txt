add_executable(SyncTests SyncTests.cpp)

add_test(sync_tests SyncTests)

target_link_libraries(SyncTests
    PRIVATE 
        GTest::gtest 
        GTest::gtest_main
        mob::sync
)