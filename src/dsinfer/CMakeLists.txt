if(DSINFER_BUILD_STATIC)
    add_library(${PROJECT_NAME} STATIC)
    target_compile_definitions(${PROJECT_NAME} PUBLIC DSINFER_STATIC)
else()
    add_library(${PROJECT_NAME} SHARED)
endif()

target_compile_definitions(${PROJECT_NAME} PRIVATE DSINFER_LIBRARY)

file(GLOB_RECURSE _src *.h *.cpp)
target_sources(${PROJECT_NAME} PRIVATE ${_src})

target_include_directories(${PROJECT_NAME} PRIVATE .)

target_include_directories(${PROJECT_NAME} PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>"
)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)

if(DSINFER_INSTALL)
    target_include_directories(${PROJECT_NAME} PUBLIC
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
    )

    install(TARGETS ${PROJECT_NAME}
        EXPORT ${DSINFER_INSTALL_NAME}Targets
        RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}" OPTIONAL
        LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}" OPTIONAL
        ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}" OPTIONAL
    )

    # Install headers
    install(DIRECTORY ./
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${DSINFER_INSTALL_NAME}
        FILES_MATCHING PATTERN *.h
                       EXCLUDE PATTERN json/*
    )
endif()