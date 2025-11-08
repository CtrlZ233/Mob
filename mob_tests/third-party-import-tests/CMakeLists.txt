add_executable(ImportTest ImportTest.cpp)

add_test(import_test ImportTest)

target_link_libraries(ImportTest
    PRIVATE 
        GTest::gtest 
        GTest::gtest_main
        mob::spdlog_header
        mob::fmt_header
)