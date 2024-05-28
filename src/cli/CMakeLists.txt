project(dsinfer-cli VERSION ${DSINFER_VERSION} LANGUAGES CXX)

file(GLOB_RECURSE _src *.h *.cpp)
add_executable(${PROJECT_NAME} ${_src})

find_package(syscmdline CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE dsinfer::Core syscmdline::syscmdline)

target_include_directories(${PROJECT_NAME} PRIVATE ${DSINFER_BUILD_INCLUDE_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE .)

if(DSINFER_INSTALL)
    install(TARGETS ${PROJECT_NAME}
        EXPORT ${DSINFER_INSTALL_NAME}Targets
        RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}" OPTIONAL
    )
endif()