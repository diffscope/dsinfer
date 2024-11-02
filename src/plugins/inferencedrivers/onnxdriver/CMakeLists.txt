project(onnxdriver
    VERSION ${DSINFER_VERSION}
    LANGUAGES C CXX
)

file(GLOB_RECURSE _src *.h *.c *.cpp)

# Import onnxruntime
set(_onnxruntime_dir ${DSINFER_SOURCE_DIR}/libs/onnxruntime)

if(NOT IS_DIRECTORY ${_onnxruntime_dir})
    return()
endif()

option(ONNXDRIVER_ENABLE_DML "Enable DirectML execution provider" OFF)
option(ONNXDRIVER_ENABLE_CUDA "Enable CUDA execution provider" OFF)

if (ONNXDRIVER_ENABLE_DML)
    set(_onnxdriver_ep_macro_dml "ONNXDRIVER_ENABLE_DML")
endif()

if (ONNXDRIVER_ENABLE_CUDA)
    set(_onnxdriver_ep_macro_cuda "ONNXDRIVER_ENABLE_CUDA")
endif()

set(_onnxdriver_ep_macros ${_onnxdriver_ep_macro_dml} ${_onnxdriver_ep_macro_cuda})

dsinfer_add_plugin(${PROJECT_NAME} ${CURRENT_PLUGIN_CATEGORY}
    SOURCES ${_src}
    FEATURES cxx_std_17
    LINKS dsinfer
    INCLUDE ${_onnxruntime_dir}/include
    INCLUDE_PRIVATE *
    PREFIX DSINFER_CORE
    DEFINES ORT_API_MANUAL_INIT ${_onnxdriver_ep_macros}
)

if(WIN32)
    # Copy onnxruntime dlls
    qm_add_copy_command(${PROJECT_NAME} # SKIP_INSTALL
        SOURCES ${_onnxruntime_dir}/lib/*.dll
        DESTINATION runtimes
        INSTALL_DIR ${CMAKE_INSTALL_PREFIX}
    )
endif()