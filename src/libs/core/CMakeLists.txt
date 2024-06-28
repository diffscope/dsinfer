project(dsinferCore
    VERSION ${DSINFER_VERSION}
    LANGUAGES CXX
)

find_package(loadso CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

file(GLOB_RECURSE _src *.h *.cpp)

dsinfer_add_library(${PROJECT_NAME} SHARED
    SOURCES ${_src}
    FEATURES cxx_std_17
    LINKS_PRIVATE loadso::loadso nlohmann_json::nlohmann_json
    INCLUDE_PRIVATE *
    PREFIX DSINFER_CORE
    SYNC_INCLUDE_OPTIONS
    EXCLUDE "internal/.+"
)