qm_import(Preprocess)

set(DSINFER_PROJECT_DESCRIPTION "DiffSinger Inference Framework")
set(DSINFER_PROJECT_COPYRIGHT "Copyright 2023-2024 Team OpenVPI")
set(DSINFER_GENERATED_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/../include)
set(DSINFER_BUILD_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/../etc/include)

# ----------------------------------
# Configurations
# ----------------------------------
set(QMSETUP_DEFINITION_SCOPE DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
set(QMSETUP_DEFINITION_NUMERICAL off)

qm_add_definition(DSINFER_TEST_CONFIG)
qm_add_definition(DSINFER_ENABLE_DIRECTML CONDITION DSINFER_ENABLE_DIRECTML)
qm_add_definition(DSINFER_ENABLE_CUDA CONDITION DSINFER_ENABLE_CUDA)

qm_generate_config(${DSINFER_BUILD_INCLUDE_DIR}/dsinfer/dsinfer_config.h)

if(DSINFER_INSTALL)
    install(FILES ${DSINFER_BUILD_INCLUDE_DIR}/dsinfer/dsinfer_config.h
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${DSINFER_INSTALL_NAME}/dsinfer
    )
endif()

# ----------------------------------
# CMake API
# ----------------------------------
macro(dsinfer_add_library _target)
    set(options SHARED NO_SYNC_INCLUDE NO_WIN_RC NO_EXPORT)
    set(oneValueArgs SYNC_INCLUDE_PREFIX PREFIX)
    set(multiValueArgs SYNC_INCLUDE_OPTIONS)
    cmake_parse_arguments(FUNC "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(FUNC_SHARED)
        set(_type SHARED)
    else()
        set(_type STATIC)
    endif()

    add_library(${_target} ${_type})

    if(WIN32 AND NOT FUNC_NO_WIN_RC)
        qm_add_win_rc(${_target}
            NAME ${DSINFER_INSTALL_NAME}
            DESCRIPTION ${DSINFER_PROJECT_DESCRIPTION}
            COPYRIGHT ${DSINFER_PROJECT_COPYRIGHT}
        )
    endif()

    if(FUNC_PREFIX)
        set(_prefix_option PREFIX ${FUNC_PREFIX})
    else()
        set(_prefix_option)
    endif()

    # Set global definitions
    qm_export_defines(${_target} ${_prefix_option})

    # Configure target
    qm_configure_target(${_target} ${FUNC_UNPARSED_ARGUMENTS})

    # Add include directories
    target_include_directories(${_target} PRIVATE ${DSINFER_BUILD_INCLUDE_DIR})
    target_include_directories(${_target} PRIVATE .)

    # Library name
    if(_target MATCHES "^dsinfer(.+)")
        set(_name ${CMAKE_MATCH_1})
        set_target_properties(${_target} PROPERTIES EXPORT_NAME ${_name})
    else()
        set(_name ${_target})
    endif()

    add_library(${DSINFER_INSTALL_NAME}::${_name} ALIAS ${_target})

    if(FUNC_SYNC_INCLUDE_PREFIX)
        set(_inc_name ${FUNC_SYNC_INCLUDE_PREFIX})
    else()
        set(_inc_name ${_target})
    endif()

    set(_install_options)

    if(DSINFER_INSTALL)
        if(FUNC_NO_EXPORT)
            set(_export)
        else()
            set(_export EXPORT ${DSINFER_INSTALL_NAME}Targets)
        endif()

        install(TARGETS ${_target}
            ${_export}
            RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}" OPTIONAL
            LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}" OPTIONAL
            ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}" OPTIONAL
        )

        target_include_directories(${_target} PUBLIC
            "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/${DSINFER_INSTALL_NAME}>"
        )

        set(_install_options
            INSTALL_DIR "${CMAKE_INSTALL_INCLUDEDIR}/${DSINFER_INSTALL_NAME}/${_inc_name}"
        )
    endif()

    if(NOT FUNC_NO_SYNC_INCLUDE)
        # Generate a standard include directory in build directory
        qm_sync_include(. "${DSINFER_GENERATED_INCLUDE_DIR}/${_inc_name}" ${_install_options}
            ${FUNC_SYNC_INCLUDE_OPTIONS} FORCE
        )
        target_include_directories(${_target} PUBLIC
            "$<BUILD_INTERFACE:${DSINFER_GENERATED_INCLUDE_DIR}>"
        )
    endif()
endmacro()

macro(dsinfer_add_plugin _target _category)
    set(options NO_WIN_RC)
    set(oneValueArgs)
    set(multiValueArgs)
    cmake_parse_arguments(FUNC "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    add_library(${_target} SHARED)

    if(WIN32 AND NOT FUNC_NO_WIN_RC)
        qm_add_win_rc(${_target}
            NAME ${DSINFER_INSTALL_NAME}
            DESCRIPTION ${DSINFER_PROJECT_DESCRIPTION}
            COPYRIGHT ${DSINFER_PROJECT_COPYRIGHT}
        )
    endif()

    # Configure target
    qm_configure_target(${_target} ${FUNC_UNPARSED_ARGUMENTS})

    # Add include directories
    target_include_directories(${_target} PRIVATE ${DSINFER_BUILD_INCLUDE_DIR})
    target_include_directories(${_target} PRIVATE .)

    # Library name
    if(_target MATCHES "^dsinfer(.+)")
        set(_name ${CMAKE_MATCH_1})
        set_target_properties(${_target} PROPERTIES EXPORT_NAME ${_name})
    else()
        set(_name ${_target})
    endif()

    add_library(${DSINFER_INSTALL_NAME}::${_name} ALIAS ${_target})

    set(_plugin_dir ../lib/plugins/${DSINFER_INSTALL_NAME}/${_category})

    set_target_properties(${_target} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${_plugin_dir}
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${_plugin_dir}
        ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${_plugin_dir}
    )

    if(DSINFER_INSTALL)
        install(TARGETS ${_target}
            RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}/${_plugin_dir}" OPTIONAL
            LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}/${_plugin_dir}" OPTIONAL
        )
    endif()
endmacro()

add_subdirectory(libs)

add_subdirectory(plugins)

add_subdirectory(apps)

if(DSINFER_BUILD_TESTS)
    add_subdirectory(tests)
endif()

if(DSINFER_INSTALL)
    qm_basic_install(
        NAME ${DSINFER_INSTALL_NAME}
        VERSION ${DSINFER_VERSION}
        INSTALL_DIR ${CMAKE_INSTALL_LIBDIR}/cmake/${DSINFER_INSTALL_NAME}
        CONFIG_TEMPLATE "${DSINFER_INSTALL_NAME}Config.cmake.in"
        NAMESPACE ${DSINFER_INSTALL_NAME}::
        EXPORT ${DSINFER_INSTALL_NAME}Targets
        WRITE_CONFIG_OPTIONS NO_CHECK_REQUIRED_COMPONENTS_MACRO
    )

    # Install copyright
    install(FILES ../LICENSE
        DESTINATION ${CMAKE_INSTALL_DATADIR}/${DSINFER_INSTALL_NAME}
    )

    if(WIN32)
        set(_lib_dir bin)
    else()
        set(_lib_dir lib)
    endif()

    if(WIN32)
        set(_extra_search_path ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
    else()
        set(_extra_search_path ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
    endif()

    qm_import(Deploy)
    qm_deploy_directory(${CMAKE_INSTALL_PREFIX}
        LIBRARY_DIR ${_lib_dir}
        EXTRA_SEARCHING_PATHS ${_extra_search_path}
        VERBOSE
    )

    # Install vcruntime
    if(MSVC)
        set(CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION "${OUTPUT_DIR}")
        include(InstallRequiredSystemLibraries)
    endif()
endif()