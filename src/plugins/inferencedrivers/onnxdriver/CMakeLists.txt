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

if (DSINFER_ENABLE_DIRECTML)
    set(_onnxdriver_ep_macro_dml "ONNXDRIVER_ENABLE_DML")
endif()

if (DSINFER_ENABLE_CUDA)
    set(_onnxdriver_ep_macro_cuda "ONNXDRIVER_ENABLE_CUDA")
endif()

set(_onnxdriver_ep_macros ${_onnxdriver_ep_macro_dml} ${_onnxdriver_ep_macro_cuda})

find_package(stduuid CONFIG REQUIRED)

dsinfer_add_plugin(${PROJECT_NAME} ${CURRENT_PLUGIN_CATEGORY}
    SOURCES ${_src}
    FEATURES cxx_std_17
    LINKS dsinfer stduuid
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