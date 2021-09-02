# Provide factory names as list of arguments to use for later
macro( PangolinRegisterFactory interface_name)
    list(APPEND PANGO_FACTORY_INTERFACES "${interface_name}")
    list(REMOVE_DUPLICATES PANGO_FACTORY_INTERFACES)
    foreach(arg ${ARGN})
        list(APPEND PANGO_FACTORY_${interface_name} "${arg}")
    endforeach()
endmacro()

# Actually Create the method call file
macro(create_factory_registry_file_now filename namespace interface_name)
    list(APPEND factory_names ${ARGN})

    file(WRITE ${filename} "// CMake generated file. Do Not Edit.\n\n#pragma once\n\nnamespace ${namespace} {\n\n")

    file(APPEND ${filename} "  // Forward declarations\n")
    foreach(factory_name ${factory_names})
#        file(APPEND ${filename} "  bool Load${factory_name}Success;\n")
        file(APPEND ${filename} "  bool Register${factory_name}Factory();\n")
    endforeach()

    file(APPEND ${filename} "\n\n")

    file(APPEND ${filename} "  inline bool RegisterFactories${interface_name}() {\n")
    file(APPEND ${filename} "    bool success = true;\n")

    foreach(factory_name ${factory_names})
#        file(APPEND ${filename} "    success &= Load${factory_name}Success;\n")
        file(APPEND ${filename} "    success &= Register${factory_name}Factory();\n")
    endforeach()
    file(APPEND ${filename} "    return success;\n")
    file(APPEND ${filename} "  }\n")

    file(APPEND ${filename} "\n\n} // ${namespace}\n")
endmacro()

# Sets up target for creating the file.
macro(create_factory_registry_file directory interface_name)
    set(filename "${directory}/RegisterFactories${interface_name}.h")
    add_custom_command(
        OUTPUT ${filename}
        DEPENDS ${PROJECT_SOURCE_DIR}/cmake/PangolinFactory.cmake
        COMMAND
            ${CMAKE_COMMAND} -DFILENAME=${filename}
                             -DNAMESPACE=pangolin
                             -DINTERFACE_NAME=${interface_name}
                             "\"-DFACTORY_NAMES=${PANGO_FACTORY_${interface_name}}\""
                             -P ${PROJECT_SOURCE_DIR}/cmake/PangolinFactory.cmake
        COMMENT "Creating ${filename} factory registry header"
    )
endmacro()

if(CMAKE_SCRIPT_MODE_FILE AND NOT CMAKE_PARENT_LIST_FILE)
    # Running in script mode as part of build-time procedure to actually to the thing
    create_factory_registry_file_now(${FILENAME} ${NAMESPACE} ${INTERFACE_NAME} ${FACTORY_NAMES} )
endif()
