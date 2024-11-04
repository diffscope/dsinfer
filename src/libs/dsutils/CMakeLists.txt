project(dsutils
    VERSION ${DSINFER_VERSION}
    LANGUAGES CXX
)

file(GLOB_RECURSE _src *.h *.cpp)

dsinfer_add_library(${PROJECT_NAME} SHARED
    SOURCES ${_src}
    FEATURES cxx_std_17
    LINKS dsinfer
    INCLUDE_PRIVATE *
    PREFIX DSUTILS
    SYNC_INCLUDE_OPTIONS
    EXCLUDE "internal/.+"
)