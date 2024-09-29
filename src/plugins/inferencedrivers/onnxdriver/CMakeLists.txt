project(onnxdriver
    VERSION ${DSINFER_VERSION}
    LANGUAGES CXX
)

file(GLOB_RECURSE _src *.h *.cpp)

dsinfer_add_plugin(${PROJECT_NAME} ${CURRENT_PLUGIN_CATEGORY}
    SOURCES ${_src}
    FEATURES cxx_std_17
    LINKS dsinfer
    INCLUDE_PRIVATE *
    PREFIX DSINFER_CORE
)