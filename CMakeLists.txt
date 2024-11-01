cmake_minimum_required(VERSION 3.19)

project(dsinfer VERSION 0.0.1.0 LANGUAGES CXX)

# ----------------------------------
# Build Options
# ----------------------------------
option(DSINFER_INSTALL "Install library" ON)
option(DSINFER_ENABLE_DIRECTML "Enable DirectML provider" OFF)
option(DSINFER_ENABLE_CUDA "Enable CUDA provider" OFF)

# ----------------------------------
# CMake Settings
# ----------------------------------
if(MSVC)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /manifest:no")
    set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} /manifest:no")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /manifest:no")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8")

    if(NOT DEFINED CMAKE_DEBUG_POSTFIX)
        set(CMAKE_DEBUG_POSTFIX "d")
    endif()
elseif(MINGW)
    set(CMAKE_STATIC_LIBRARY_PREFIX "")
    set(CMAKE_SHARED_LIBRARY_PREFIX "")
endif()

if(DSINFER_INSTALL)
    include(GNUInstallDirs)
    include(CMakePackageConfigHelpers)
endif()

# ----------------------------------
# Project Variables
# ----------------------------------
set(DSINFER_VERSION ${PROJECT_VERSION})
set(DSINFER_INSTALL_NAME ${PROJECT_NAME})
set(DSINFER_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# ----------------------------------
# Find basic dependencies
# ----------------------------------
find_package(qmsetup REQUIRED)

qm_import(Filesystem)
qm_init_directories()

# ----------------------------------
# Add source modules
# ----------------------------------
add_subdirectory(src)