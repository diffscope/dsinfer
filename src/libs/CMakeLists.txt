add_subdirectory(core)

if(IS_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/csharp)
    add_subdirectory(csharp)
endif()