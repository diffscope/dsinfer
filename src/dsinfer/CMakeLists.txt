file(GLOB_RECURSE _src *.h *.cpp)

dsinfer_add_library(${PROJECT_NAME} AUTOGEN
    SOURCES ${_src}
    FEATURES cxx_std_17
    INCLUDE_PRIVATE inference
)