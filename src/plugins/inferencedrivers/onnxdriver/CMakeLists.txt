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

dsinfer_add_plugin(${PROJECT_NAME} ${CURRENT_PLUGIN_CATEGORY}
    SOURCES ${_src}
    FEATURES cxx_std_17
    LINKS dsinfer
    INCLUDE ${_onnxruntime_dir}/include
    INCLUDE_PRIVATE *
    PREFIX DSINFER_CORE
    DEFINES ORT_API_MANUAL_INIT
)

if(WIN32)
    # Copy onnxruntime dlls
    qm_add_copy_command(${PROJECT_NAME} # SKIP_INSTALL
        SOURCES ${_onnxruntime_dir}/lib/*.dll
        DESTINATION runtimes
        INSTALL_DIR ${CMAKE_INSTALL_PREFIX}
    )
endif()