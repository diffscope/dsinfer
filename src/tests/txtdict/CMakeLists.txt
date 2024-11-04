project(tst_txtdict)

file(GLOB _src *.h *.cpp)
add_executable(${PROJECT_NAME} ${_src})
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)
target_link_libraries(${PROJECT_NAME} PRIVATE dsutils)