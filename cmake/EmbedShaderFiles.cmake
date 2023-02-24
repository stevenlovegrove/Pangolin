# Creates C resources file from specified files
# Based on http://stackoverflow.com/a/27206982
function(embed_shader_files_now output escaped_shader_files project_prefix)
    # Undo the hack we did within embed_shader_files_rule
    string(REGEX REPLACE "," ";" shader_files "${escaped_shader_files}")

    # Create empty output file
    file(WRITE ${output} "")

    # Add header
    file(APPEND ${output} "#include <unordered_map>\n")
    file(APPEND ${output} "#include <string>\n\n")
    file(APPEND ${output} "namespace pangolin {\n")
    file(APPEND ${output} "const std::unordered_map<std::string,const char*>& GetBuiltinShaders() {\n")


    file(APPEND ${output} "\tstatic const std::unordered_map<std::string,const char*> builtin_shaders = {\n")

    # Iterate through input files
    foreach(shader ${shader_files})
        # Get short filename
        STRING(REGEX REPLACE "^${project_prefix}" "" filename ${shader})

        # Read data from file
        file(READ ${shader} filedata)

        # Append data to output file
        file(APPEND ${output} "{ \"${filename}\",\n R\"SHADER_SOURCE(\n")
        file(APPEND ${output} "${filedata}")
        file(APPEND ${output} ")SHADER_SOURCE\" },\n")
    endforeach()

    # Add footer
    file(APPEND ${output} "\t};\n")
    file(APPEND ${output} "\treturn builtin_shaders;\n")
    file(APPEND ${output} "}\n")
    file(APPEND ${output} "}\n")

endfunction()

# Sets up rule for embedding files when they are newer than the output file (or it doesn't exist yet)
function(embed_shader_files_rule output shader_files)
    # Hack so that add_custom_command doesn't do something silly with the semi-colons.
    string(REGEX REPLACE ";" "," escaped_shader_files "${shader_files}")

    add_custom_command(
        OUTPUT ${output}
        DEPENDS ${shader_files} ${PROJECT_SOURCE_DIR}/cmake/EmbedShaderFiles.cmake
        COMMAND
            ${CMAKE_COMMAND} -DINPUT_SHADER_FILES="${escaped_shader_files}"
                             -DOUTPUT_SRC_FILE=${output}
                             -DPREFIX_TO_STRIP=${PROJECT_SOURCE_DIR}
                             -P ${PROJECT_SOURCE_DIR}/cmake/EmbedShaderFiles.cmake
        COMMENT "Embedding ${shader_files} into ${output}"
    )
endfunction()

if(CMAKE_SCRIPT_MODE_FILE AND NOT CMAKE_PARENT_LIST_FILE)
    # Running in script mode as part of build-time procedure to actually to the embedding
    embed_shader_files_now("${OUTPUT_SRC_FILE}" "${INPUT_SHADER_FILES}" "${PREFIX_TO_STRIP}" )
endif()
