project(tst_onnxdriver)

if(NOT TARGET onnxdriver)
    return()
endif()

file(GLOB _src *.h *.cpp)
add_executable(${PROJECT_NAME} ${_src})
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)

target_link_libraries(${PROJECT_NAME} PRIVATE dsinfer onnxdriver)

# Add a custom command to copy the directory after building
qm_add_copy_command(${PROJECT_NAME} SKIP_INSTALL
    SOURCES test_data
    DESTINATION .
)