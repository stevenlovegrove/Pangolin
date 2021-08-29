# Creates C resources file from specified files
# Based on http://stackoverflow.com/a/27206982
function(embed_binary_files_now output binary_files)
    # Create empty output file
    file(WRITE ${output} "")
    # Iterate through input files
    foreach(bin ${binary_files})
        # Get short filename
        string(REGEX MATCH "([^/]+)$" filename ${bin})
        # Replace filename spaces & extension separator for C compatibility
        string(REGEX REPLACE "\\.| " "_" filename ${filename})
        # Read hex data from file
        file(READ ${bin} filedata HEX)
        # Convert hex data for C compatibility
        string(REGEX REPLACE "([0-9a-f][0-9a-f])" "0x\\1," filedata ${filedata})
        # Append data to output file
        file(APPEND ${output} "extern const unsigned char ${filename}[] = {${filedata}};\nextern const unsigned ${filename}_size = sizeof(${filename});\n")
    endforeach()
endfunction()

# Sets up rule for embedding files when they are newer than the output file (or it doesn't exist yet)
function(embed_binary_files_rule output binary_files)
    add_custom_command(
        OUTPUT ${output}
        DEPENDS ${binary_files}
        COMMAND
            ${CMAKE_COMMAND} -DINPUT_BINARY_FILES=${binary_files}
                             -DOUTPUT_SRC_FILE=${output}
                             -P ${PROJECT_SOURCE_DIR}/cmake/EmbedBinaryFiles.cmake
        COMMENT "Embedding ${binary_files} into ${output}"
    )
endfunction()

if(CMAKE_SCRIPT_MODE_FILE AND NOT CMAKE_PARENT_LIST_FILE)
    # Running in script mode as part of build-time procedure to actually to the embedding
    embed_binary_files_now(${OUTPUT_SRC_FILE} ${INPUT_BINARY_FILES} )
endif()