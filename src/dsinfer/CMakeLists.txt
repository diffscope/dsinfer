qm_import(Filesystem)

file(GLOB_RECURSE _src *.h *.cpp)

find_package(syscmdline CONFIG REQUIRED)

dsinfer_add_library(${PROJECT_NAME} AUTOGEN
    SOURCES ${_src}
    FEATURES cxx_std_17
    INCLUDE_PRIVATE inference
    LINKS_PRIVATE
        syscmdline::syscmdline
    DEFINES
        ORT_API_MANUAL_INIT
)

# Link onnxruntime
if(TRUE)
    target_include_directories(${PROJECT_NAME} PRIVATE ../../libs/onnxruntime/include)
    target_link_directories(${PROJECT_NAME} PRIVATE ../../libs/onnxruntime/lib)
endif()