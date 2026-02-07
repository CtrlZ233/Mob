include(${PROJECT_SOURCE_DIR}/cmake/mob_lib.cmake)

mob_library(INTERFACE
    NAME
        task
    DEPS
        mob::sync
        mob::compiler
        mob::concurrentqueue
    INCLUDES
        ${PROJECT_SOURCE_DIR}
)
