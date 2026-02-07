include(${PROJECT_SOURCE_DIR}/cmake/mob_lib.cmake)

mob_library(INTERFACE
    NAME
        compiler
    INCLUDES
        ${PROJECT_SOURCE_DIR}
)