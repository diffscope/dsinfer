project(dsinfer-cli VERSION ${DSINFER_VERSION} LANGUAGES CXX)

file(GLOB_RECURSE _src *.h *.cpp)

add_executable(${PROJECT_NAME} ${_src})

target_link_libraries(${PROJECT_NAME} PRIVATE dsinfer)