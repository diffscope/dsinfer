project(dsutils
    VERSION ${DSINFER_VERSION}
    LANGUAGES CXX
)

file(GLOB_RECURSE _src *.h *.cpp)

dsinfer_add_library(${PROJECT_NAME} SHARED
    SOURCES ${_src}
    FEATURES cxx_std_17
    LINKS dsinfer
    LINKS_PRIVATE
    INCLUDE_PRIVATE *
    PREFIX DSUTILS
    SYNC_INCLUDE_OPTIONS
    EXCLUDE "internal/.+"
)

find_path(SPARSEPP_INCLUDE_DIRS "sparsepp/spp.h")
target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${SPARSEPP_INCLUDE_DIRS}>
)