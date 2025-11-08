# mob_library函数：用于定义Mob项目中的库目标
#
# 参数:
#   [STATIC|SHARED|INTERFACE] - 库类型（可选，默认为STATIC）
#   NAME <name>               - 库名称（必需）
#   HDRS <header1> ...        - 公共头文件列表
#   SRCS <source1> ...        - 源文件列表（INTERFACE库不需要）
#   DEPS <target1> ...        - 依赖目标列表
#   INCLUDES <dir1> ...       - 额外的包含目录
#   COMPILE_OPTIONS <option1> ... - 编译选项
#   COMPILE_DEFINITIONS <def1> ... - 编译定义

function(mob_library)
    set(option_args 
        STATIC 
        SHARED 
        INTERFACE
    )
    set(one_value_args 
        NAME
    )
    set(multi_value_args 
        HDRS 
        SRCS 
        DEPS 
        INCLUDES
        COMPILE_OPTIONS
        COMPILE_DEFINITIONS
    )
    
    cmake_parse_arguments(MOB_LIB
        "${option_args}"
        "${one_value_args}"
        "${multi_value_args}"
        ${ARGN}
    )

    if(NOT MOB_LIB_NAME)
        message(FATAL_ERROR "mob_library: NAME argument is required")
    endif()

    set(lib_type "STATIC")
    set(visibility "PUBLIC")
    if(MOB_LIB_STATIC)
        set(lib_type "STATIC")
    elseif(MOB_LIB_SHARED)
        set(lib_type "SHARED")
    elseif(MOB_LIB_INTERFACE)
        set(lib_type "INTERFACE")
        set(visibility "INTERFACE")
    endif()

    if(lib_type STREQUAL "INTERFACE" AND MOB_LIB_SRCS)
        message(WARNING "mob_library: INTERFACE library '${MOB_LIB_NAME}' should not have SRCS")
    elseif(NOT lib_type STREQUAL "INTERFACE" AND NOT MOB_LIB_SRCS)
        message(FATAL_ERROR "mob_library: Non-INTERFACE library '${MOB_LIB_NAME}' requires SRCS")
    endif()

    if(lib_type STREQUAL "INTERFACE")
        add_library(${MOB_LIB_NAME} INTERFACE)
    else()
        add_library(${MOB_LIB_NAME} ${lib_type} ${MOB_LIB_SRCS})
    endif()

    if(MOB_LIB_HDRS)
        target_sources(${MOB_LIB_NAME}
            PUBLIC 
                FILE_SET HEADERS
                TYPE HEADERS
                FILES ${MOB_LIB_HDRS}
        )
    endif()

    if(MOB_LIB_INCLUDES)
        target_include_directories(${MOB_LIB_NAME}
            ${visibility}
            ${MOB_LIB_INCLUDES}
        )
    endif()

    target_include_directories(${MOB_LIB_NAME}
        ${visibility}
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
    )

    if(MOB_LIB_DEPS)
        target_link_libraries(${MOB_LIB_NAME}
            ${lib_type}
            ${MOB_LIB_DEPS}
        )
    endif()

    if(MOB_LIB_COMPILE_OPTIONS)
        target_compile_options(${MOB_LIB_NAME}
            ${lib_type}
            ${MOB_LIB_COMPILE_OPTIONS}
        )
    endif()

    if(MOB_LIB_COMPILE_DEFINITIONS)
        target_compile_definitions(${MOB_LIB_NAME}
            ${lib_type}
            ${MOB_LIB_COMPILE_DEFINITIONS}
        )
    endif()

    set_target_properties(${MOB_LIB_NAME} PROPERTIES
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR}
        OUTPUT_NAME "${MOB_LIB_NAME}"
        DEBUG_POSTFIX "d"
    )

    add_library(mob::${MOB_LIB_NAME} ALIAS ${MOB_LIB_NAME})
    
    message(STATUS "Created Mob library: ${MOB_LIB_NAME} (${lib_type})")
    
endfunction()