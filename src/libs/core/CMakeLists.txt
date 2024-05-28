project(dsinferCore
    VERSION ${DSINFER_VERSION}
    LANGUAGES CXX
)

find_package(loadso CONFIG REQUIRED)

file(GLOB_RECURSE _src *.h *.cpp)

dsinfer_add_library(${PROJECT_NAME} SHARED
    SOURCES ${_src}
    FEATURES cxx_std_17
    LINKS_PRIVATE
    loadso::loadso
    INCLUDE_PRIVATE *
    PREFIX DSINFER_CORE
    DEFINES
    ORT_API_MANUAL_INIT
    SYNC_INCLUDE_OPTIONS
    EXCLUDE "internal/.+"
)

target_include_directories(${PROJECT_NAME} PRIVATE ../../../libs/onnxruntime/include)
target_link_directories(${PROJECT_NAME} PRIVATE ../../../libs/onnxruntime/lib)

if(WIN32)
    qm_import(Filesystem)
    qm_add_copy_command(${PROJECT_NAME}
        SOURCES ../../../libs/onnxruntime/lib/*.dll
    )
endif()