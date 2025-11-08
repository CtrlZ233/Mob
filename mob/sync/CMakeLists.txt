include(${PROJECT_SOURCE_DIR}/cmake/mob_lib.cmake)

mob_library(
    NAME
        sync
    SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/SyncPoint.cpp
    INCLUDES
        ${PROJECT_SOURCE_DIR}
)