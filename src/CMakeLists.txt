add_subdirectory(dsinfer)

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