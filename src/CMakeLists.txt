qm_import(Preprocess)

set(DSINFER_PROJECT_DESCRIPTION "DiffScope Inference Midware")
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
    if(${_target} MATCHES "^dsinfer(.+)")
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

add_subdirectory(libs)

add_subdirectory(plugins)

# add_subdirectory(cli)

if(DSINFER_INSTALL)
    # Add install target
    set(_install_dir ${CMAKE_INSTALL_LIBDIR}/cmake/${DSINFER_INSTALL_NAME})

    # Add version file
    write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/${DSINFER_INSTALL_NAME}ConfigVersion.cmake"
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY AnyNewerVersion
    )

    # Add configuration file
    configure_package_config_file(
        ${CMAKE_CURRENT_LIST_DIR}/${DSINFER_INSTALL_NAME}Config.cmake.in
        "${CMAKE_CURRENT_BINARY_DIR}/${DSINFER_INSTALL_NAME}Config.cmake"
        INSTALL_DESTINATION ${_install_dir}
        NO_CHECK_REQUIRED_COMPONENTS_MACRO
    )

    # Install cmake files
    install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/${DSINFER_INSTALL_NAME}Config.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/${DSINFER_INSTALL_NAME}ConfigVersion.cmake"
        DESTINATION ${_install_dir}
    )

    # Install cmake targets files
    install(EXPORT ${DSINFER_INSTALL_NAME}Targets
        FILE "${DSINFER_INSTALL_NAME}Targets.cmake"
        NAMESPACE ${DSINFER_INSTALL_NAME}::
        DESTINATION ${_install_dir}
    )
endif()