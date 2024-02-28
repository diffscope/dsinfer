qm_import(Filesystem)

file(GLOB_RECURSE _src *.h *.cpp)

dsinfer_add_library(${PROJECT_NAME} AUTOGEN
    SOURCES ${_src}
    FEATURES cxx_std_17
    INCLUDE_PRIVATE inference
)

# Link onnxruntime
if(TRUE)
    target_include_directories(${PROJECT_NAME} PRIVATE ../../libs/onnxruntime/include)
    target_link_directories(${PROJECT_NAME} PRIVATE ../../libs/onnxruntime/lib)
endif()