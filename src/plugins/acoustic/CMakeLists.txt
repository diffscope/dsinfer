project(acoustic
    VERSION ${DSINFER_VERSION}
    LANGUAGES CXX
)

file(GLOB_RECURSE _src *.h *.cpp)

dsinfer_add_library(${PROJECT_NAME} SHARED
    SOURCES ${_src}
    FEATURES cxx_std_17
    LINKS dsinfer::Core
    INCLUDE_PRIVATE *
    PREFIX DSINFER_CORE
)