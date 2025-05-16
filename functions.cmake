# functions.cmake

# Function to copy files
function(copy_files_post_build project_name files target_dir)
    foreach(file ${files})
        # Get the file name from the path
        get_filename_component(dest_file ${file} NAME)
        set(dest_path "${target_dir}/${dest_file}")

        # Copy the file to the target directory
        add_custom_command(TARGET ${project_name} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy ${file} ${dest_path}
        )
    endforeach()
endfunction()

# Function to copy directories
function(copy_dirs_post_build project_name source_dirs target_dir)
    foreach(source_dir ${source_dirs})
        # Extract the directory name from the full path
        get_filename_component(dir_name ${source_dir} NAME)
        # Create the full destination path
        set(dest_dir "${target_dir}/${dir_name}")

        # Copy the source directory to the target directory
        add_custom_command(TARGET ${project_name} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory ${dest_dir}
            COMMAND ${CMAKE_COMMAND} -E copy_directory ${source_dir} ${dest_dir}
        )
    endforeach()
endfunction()
