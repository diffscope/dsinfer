project(onnxdriver
    VERSION ${DSINFER_VERSION}
    LANGUAGES CXX
)

file(GLOB_RECURSE _src *.h *.cpp)

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
find_package(unofficial-hash-library CONFIG REQUIRED)

dsinfer_add_plugin(${PROJECT_NAME} ${CURRENT_PLUGIN_CATEGORY}
    SOURCES ${_src}
    FEATURES cxx_std_17
    LINKS dsinfer
    LINKS_PRIVATE stduuid unofficial::hash-library
    INCLUDE ${_onnxruntime_dir}/include
    INCLUDE_PRIVATE *
    PREFIX DSINFER_CORE
    DEFINES ORT_API_MANUAL_INIT ${_onnxdriver_ep_macros}
)

if(WIN32)
    set(_ort_lib_files ${_onnxruntime_dir}/lib/*.dll)
elseif(APPLE)
    set(_ort_lib_files ${_onnxruntime_dir}/lib/*.dylib)
else()
    set(_ort_lib_files ${_onnxruntime_dir}/lib/*.so ${_onnxruntime_dir}/lib/*.so.*)
endif()

# Copy onnxruntime shared libraries
qm_add_copy_command(${PROJECT_NAME} # SKIP_INSTALL
    SOURCES ${_ort_lib_files}
    DESTINATION runtimes
    INSTALL_DIR ${CMAKE_INSTALL_PREFIX}
)