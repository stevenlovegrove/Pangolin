# Useful references:
#  https://thomastrapp.com/blog/building-a-pypi-package-for-a-modern-cpp-project/ for reference
#  https://www.benjack.io/2017/06/12/python-cpp-tests.html also interesting
#  https://www.python.org/dev/peps/pep-0427/ the official package format description

function( MakeWheel python_module)
    cmake_parse_arguments(MAKEWHEEL "PRINT_HELP" "MODULE;VERSION;SUMMARY;DESCRIPTION;HOMEPAGE;AUTHOR;EMAIL;LICENCE" "REQUIRES" ${ARGN} )
    set(version ${MAKEWHEEL_VERSION})

    ##########################################
    ## Build necessary variables for paths / variables
    string(REPLACE "-" ";" SOABI_PARTS ${Python_SOABI})
    list(GET SOABI_PARTS 0 pythontag )
    list(GET SOABI_PARTS 1 pythonversion )
    list(GET SOABI_PARTS 2 pythonplatform )

    # https://www.python.org/dev/peps/pep-0425/#python-tag
    string( TOLOWER "${pythontag}" pythontag )
    if(pythontag STREQUAL "cpython")
        set(pythontag_short "cp")
    elseif(pythontag STREQUAL "ironpython")
        set(pythontag_short "ip")
    elseif(pythontag STREQUAL "pypy")
        set(pythontag_short "pp")
    elseif(pythontag STREQUAL "jython")
        set(pythontag_short "jp")
    endif()

    ##########################################
    # execute python to get correct platform string.
    execute_process(
        COMMAND ${Python_EXECUTABLE} -c "import distutils.util; print(distutils.util.get_platform())"
        OUTPUT_VARIABLE platformtag
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    string(REPLACE "-" "_" platformtag ${platformtag})
    string(REPLACE "." "_" platformtag ${platformtag})

    # https://www.python.org/dev/peps/pep-0427/#file-format
    set(pythontag_version "${pythontag_short}${pythonversion}")
    set(complete_tag "${pythontag_version}-${pythontag_version}-${platformtag}")
    set(wheel_filename "${CMAKE_BINARY_DIR}/${python_module}-${version}-${complete_tag}.whl")
    set(wheel_distinfo "${CMAKE_BINARY_DIR}/${python_module}-${version}.dist-info")
    set(wheel_data "${CMAKE_BINARY_DIR}/${python_module}-${version}.data")
    set(wheel_generator_string "pango_wheelgen_${version}")


    if( MAKEWHEEL_REQUIRES )
        set(MAKEWHEEL_REQUIRES "Requires-Dist: ${MAKEWHEEL_REQUIRES}")
        string(REPLACE ";" "\nRequires-Dist: " MAKEWHEEL_REQUIRES "${MAKEWHEEL_REQUIRES}")
    endif()

    ##########################################
    ## Create dist info folder
    file(GLOB wheel_info_files "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/wheel-dist-info/*" )
    file(WRITE "${wheel_distinfo}/RECORD" "")
    foreach(template_path ${wheel_info_files})
        get_filename_component(template_name "${template_path}" NAME)
        configure_file( ${template_path} "${wheel_distinfo}/${template_name}" )
        file(SHA256 "${template_path}" file_sha)
        file (SIZE "${template_path}" file_size)
        file(APPEND "${wheel_distinfo}/RECORD" "${python_module}-${version}.dist-info/${template_name},sha256=${file_sha},${file_size}\n")
    endforeach()

    # TODO: Add line in RECORD for .so module itself (needs to be build time)

    ##########################################
    ## Place module into data folder
    set_target_properties( ${python_module} PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${wheel_data}/purelib")

    ##########################################
    ## Rule for creating file wheel zip
    add_custom_command(
        OUTPUT ${wheel_filename}
        DEPENDS ${python_module}
        COMMAND ${CMAKE_COMMAND} -E tar cf ${wheel_filename} --format=zip -- ${wheel_distinfo} ${wheel_data}
        COMMENT "Creating Wheel ${wheel_filename}"
        VERBATIM
    )

    ##########################################
    ## Create a friendlier target name we can refer to
    add_custom_target("${python_module}_wheel" DEPENDS "${wheel_filename}")

    ##########################################
    ## Help message since this is tricky for people
    if(MAKEWHEEL_PRINT_HELP)
        message(STATUS "Selected Python: '${Python_EXECUTABLE}'. To use module:\nBuild Wheel: cmake --build . -t ${python_module}_wheel\nInstall: ${Python_EXECUTABLE} -mpip install ${wheel_filename}\nUninstall: ${Python_EXECUTABLE} -mpip uninstall ${python_module}")
    endif()
endfunction()


