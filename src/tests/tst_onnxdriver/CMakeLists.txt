project(tst_onnxdriver)

file(GLOB _src *.h *.cpp)
add_executable(${PROJECT_NAME} ${_src})
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)

target_link_libraries(${PROJECT_NAME} PRIVATE dsinfer onnxdriver)

# Copy test data to build directory
set(DATA_DIR_NAME "test_data")
set(SOURCE_DATA_PATH "${CMAKE_CURRENT_SOURCE_DIR}/${DATA_DIR_NAME}")
set(DEST_DATA_PATH "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${DATA_DIR_NAME}")

# Normalize and use native path
# (e.g. use backslash in Windows for cmd commands compatibility)
cmake_path(NATIVE_PATH SOURCE_DATA_PATH NORMALIZE SOURCE_DATA_PATH_NORM)
cmake_path(NATIVE_PATH DEST_DATA_PATH NORMALIZE DEST_DATA_PATH_NORM)

# Add a custom command to copy the directory after building
if(WIN32)
    # Windows-specific commands
    add_custom_command(
            TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND xcopy /E /H /C /I /Y "\"${SOURCE_DATA_PATH_NORM}\\*\"" "\"${DEST_DATA_PATH_NORM}\\\""
            COMMENT "Copying test data to the build directory and overwriting existing files (Windows)"
    )
else()
    # Unix-specific commands
    add_custom_command(
            TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND cp -r "\"${SOURCE_DATA_PATH_NORM}/.\"" "\"${DEST_DATA_PATH_NORM}/\""
            COMMENT "Copying test data to the build directory and overwriting existing files (Unix)"
    )
endif()