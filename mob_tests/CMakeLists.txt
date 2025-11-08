cmake_minimum_required(VERSION 3.10)
project(MobTest)

find_package(GTest REQUIRED)

enable_testing()

set(MOB_USE_SPDLOG ON CACHE BOOL "Build the SPDLOG" FORCE)
set(MOB_USE_FMT ON CACHE BOOL "Build the FMT" FORCE)

set(MOB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..)

add_subdirectory(${MOB_DIR} mob)

add_subdirectory(third-party-import-tests)

add_subdirectory(mob-tests)

add_executable(mob_test main.cpp)

add_test(NAME MobTest COMMAND mob_test)

target_link_libraries(mob_test
    PRIVATE 
        GTest::gtest 
        GTest::gtest_main
)
