option(MOB_USE_FMT "Use system-installed fmt library" OFF)
option(MOB_USE_SPDLOG "Use system-installed spdlog library" OFF)

if(MOB_USE_FMT)
    add_library(fmt_header INTERFACE)
    target_include_directories(fmt_header INTERFACE fmt/include)
    add_library(mob::fmt_header ALIAS fmt_header)
endif()

if(MOB_USE_SPDLOG)
    add_library(spdlog_header INTERFACE)
    target_include_directories(spdlog_header INTERFACE spdlog/include)
    add_library(mob::spdlog_header ALIAS spdlog_header)
endif()