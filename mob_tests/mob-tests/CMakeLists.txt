# Cache Tests
add_executable(CacheTests CacheTests.cpp)
add_test(NAME CacheTests COMMAND CacheTests)
target_link_libraries(CacheTests
    PRIVATE
        GTest::gtest
        GTest::gtest_main
        mob::cache
)

# Compiler Tests
add_executable(CompilerTest CompilerTest.cpp)
add_test(NAME CompilerTest COMMAND CompilerTest)
target_link_libraries(CompilerTest
    PRIVATE
        GTest::gtest
        GTest::gtest_main
        mob::compiler
)

# Sync Tests
add_executable(SyncTests SyncTests.cpp)
add_test(NAME SyncTests COMMAND SyncTests)
target_link_libraries(SyncTests
    PRIVATE
        GTest::gtest
        GTest::gtest_main
        mob::sync
)

# Random Tests
add_executable(RandomTests RandomTests.cpp)
add_test(NAME RandomTests COMMAND RandomTests)
target_link_libraries(RandomTests
    PRIVATE
        GTest::gtest
        GTest::gtest_main
        mob::random
)
