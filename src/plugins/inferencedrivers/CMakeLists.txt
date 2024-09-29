get_filename_component(_dir_name ${CMAKE_CURRENT_LIST_DIR} NAME)
set(CURRENT_PLUGIN_CATEGORY ${_dir_name})

add_subdirectory(onnxdriver)