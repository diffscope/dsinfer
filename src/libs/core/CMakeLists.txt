project(dsinfer
    VERSION ${DSINFER_VERSION}
    LANGUAGES CXX
)

find_package(nlohmann_json CONFIG REQUIRED)

file(GLOB_RECURSE _src *.h *.cpp)

set(_platform_libs)

if(NOT WIN32)
    list(APPEND _platform_libs dl)
endif()

dsinfer_add_library(${PROJECT_NAME} SHARED
    SOURCES ${_src}
    FEATURES cxx_std_17
    LINKS_PRIVATE nlohmann_json::nlohmann_json ${_platform_libs}
    INCLUDE_PRIVATE *
    PREFIX DSINFER
    SYNC_INCLUDE_OPTIONS
    EXCLUDE "internal/.+"
)