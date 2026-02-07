cmake_minimum_required(VERSION 3.10)
project(MobTest)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(GTest REQUIRED)

enable_testing()

set(MOB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..)

add_subdirectory(${MOB_DIR} mob)

add_subdirectory(third-party-import-tests)

add_subdirectory(mob-tests)
