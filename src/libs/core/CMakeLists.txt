project(dsinfer
    VERSION ${DSINFER_VERSION}
    LANGUAGES CXX
)

find_package(nlohmann_json CONFIG REQUIRED)
find_package(stdcorelib CONFIG REQUIRED)

file(GLOB_RECURSE _src *.h *.cpp)

dsinfer_add_library(${PROJECT_NAME} SHARED
    SOURCES ${_src}
    FEATURES cxx_std_17
    LINKS_PRIVATE nlohmann_json::nlohmann_json
    LINKS stdcorelib::stdcorelib
    INCLUDE_PRIVATE *
    PREFIX DSINFER
    SYNC_INCLUDE_OPTIONS
    EXCLUDE "internal/.+"
)