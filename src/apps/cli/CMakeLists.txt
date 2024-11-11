project(dsinfer-cli VERSION ${DSINFER_VERSION} LANGUAGES CXX)

string(TIMESTAMP _year "%Y")
set(RC_COPYRIGHT "Copyright 2023-${_year} Team OpenVPI")
set(RC_DESCRIPTION "DiffSinger Inference Command Line Tool")

add_executable(${PROJECT_NAME})

find_package(unofficial-hash-library CONFIG REQUIRED)
find_package(syscmdline CONFIG REQUIRED)
find_package(unofficial-bit7z CONFIG REQUIRED)

file(GLOB_RECURSE _src *.h *.cpp)
qm_configure_target(${PROJECT_NAME}
    SOURCES ${_src}
    FEATURES cxx_std_17
    LINKS_PRIVATE dsinfer syscmdline::syscmdline unofficial::hash-library unofficial::bit7z::bit7z64
    INCLUDE ${_onnxruntime_dir}/include
    INCLUDE_PRIVATE * ${DSINFER_BUILD_INCLUDE_DIR}
    PREFIX DSINFER_CORE
    DEFINES ORT_API_MANUAL_INIT ${_onnxdriver_ep_macros}
)

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

    # Install 7zip runtime
    if(WIN32)
        set(_so_ext ".dll")
        set(_so_dest ${CMAKE_INSTALL_BINDIR})
    else()
        if(APPLE)
            set(_so_ext ".dylib")
        else()
            set(_so_ext ".so*")
        endif()

        set(_so_dest ${CMAKE_INSTALL_LIBDIR})
    endif()

    string(TOUPPER ${CMAKE_BUILD_TYPE} _build_type_upper)
    get_filename_component(_searching_path
        "${QMSETUP_APPLOCAL_DEPS_PATHS_${_build_type_upper}}"
        ABSOLUTE BASE_DIR ${CMAKE_SOURCE_DIR}
    )

    file(GLOB _seven_zip_lib "${_searching_path}/*7zip*${_so_ext}")
    install(FILES ${_seven_zip_lib}
        DESTINATION ${_so_dest}
    )
endif()