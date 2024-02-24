qm_import(Preprocess)

set(DSINFER_PROJECT_DESCRIPTION "Cross-platform window customization framework")
set(DSINFER_PROJECT_COPYRIGHT "Copyright 2023 Stdware Collections")
set(DSINFER_GENERATED_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/../include)

# ----------------------------------
# CMake API
# ----------------------------------
macro(dsinfer_add_library _target)
    set(options NO_SYNC_INCLUDE NO_WIN_RC)
    set(oneValueArgs SYNC_INCLUDE_PREFIX PREFIX)
    set(multiValueArgs SYNC_INCLUDE_OPTIONS)
    cmake_parse_arguments(FUNC "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(DSINFER_BUILD_STATIC)
        set(_type STATIC)
    else()
        set(_type SHARED)
    endif()

    add_library(${_target} ${_type})

    if(WIN32 AND NOT FUNC_NO_WIN_RC AND(${_type} STREQUAL "SHARED"))
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
    target_include_directories(${_target} PRIVATE .)

    add_library(${DSINFER_INSTALL_NAME}::${_target} ALIAS ${_target})

    if(FUNC_SYNC_INCLUDE_PREFIX)
        set(_inc_name ${FUNC_SYNC_INCLUDE_PREFIX})
    else()
        set(_inc_name ${_target})
    endif()

    set(_install_options)

    if(DSINFER_INSTALL)
        install(TARGETS ${_target}
            EXPORT ${DSINFER_INSTALL_NAME}Targets
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

add_subdirectory(dsinfer)

add_subdirectory(cli)

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