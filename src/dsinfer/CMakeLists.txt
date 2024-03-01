qm_import(Filesystem)

file(GLOB_RECURSE _src *.h *.cpp)

find_package(loadso CONFIG REQUIRED)

dsinfer_add_library(${PROJECT_NAME}
    SOURCES ${_src}
    FEATURES cxx_std_17
    INCLUDE_PRIVATE
        core
        inference
        internal
    LINKS_PRIVATE
        loadso::loadso
    DEFINES
        ORT_API_MANUAL_INIT
    SYNC_INCLUDE_OPTIONS
        EXCLUDE "internal/.+"
)

# Link onnxruntime
if(TRUE)
    target_include_directories(${PROJECT_NAME} PRIVATE ../../libs/onnxruntime/include)
    target_link_directories(${PROJECT_NAME} PRIVATE ../../libs/onnxruntime/lib)
endif()