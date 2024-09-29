project(dsinfer-cli VERSION ${DSINFER_VERSION} LANGUAGES CXX)

string(TIMESTAMP _year "%Y")
set(RC_COPYRIGHT "Copyright 2023-${_year} Team OpenVPI")
set(RC_DESCRIPTION "DiffSinger Inference Command Line Tool")

file(GLOB_RECURSE _src *.h *.cpp)
add_executable(${PROJECT_NAME} ${_src})

find_package(syscmdline CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE dsinfer syscmdline::syscmdline)

target_include_directories(${PROJECT_NAME} PRIVATE ${DSINFER_BUILD_INCLUDE_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE .)

if(WIN32)
    qm_add_win_rc(${PROJECT_NAME}
        COPYRIGHT ${RC_COPYRIGHT}
        DESCRIPTION ${RC_DESCRIPTION}
    )

    # Make sure Windows UAC does not believe it is an installer
    qm_add_win_manifest(${PROJECT_NAME}
        DESCRIPTION ${RC_DESCRIPTION}
    )
endif()

target_compile_definitions(${PROJECT_NAME} PRIVATE TOOL_DESC="${RC_DESCRIPTION}, Version ${DSINFER_VERSION}")
target_compile_definitions(${PROJECT_NAME} PRIVATE TOOL_VERSION="${DSINFER_VERSION}")

if(DSINFER_INSTALL)
    install(TARGETS ${PROJECT_NAME}
        EXPORT ${DSINFER_INSTALL_NAME}Targets
        RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}" OPTIONAL
    )
endif()