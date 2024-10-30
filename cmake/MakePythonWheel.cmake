# Useful references:
#  https://thomastrapp.com/blog/building-a-pypi-package-for-a-modern-cpp-project/ for reference
#  https://www.benjack.io/2017/06/12/python-cpp-tests.html also interesting
#  https://www.python.org/dev/peps/pep-0427/ the official package format description

set(_MAKE_PYTHON_WHEEL_LIST_DIR "${CMAKE_CURRENT_LIST_DIR}")

function( MakeWheel python_module)
    cmake_parse_arguments(MAKEWHEEL "PRINT_HELP" "MODULE;VERSION;SUMMARY;DESCRIPTION;HOMEPAGE;AUTHOR;EMAIL;LICENCE" "REQUIRES" ${ARGN} )
    set(version ${MAKEWHEEL_VERSION})

    execute_process(
        COMMAND ${Python3_EXECUTABLE} -c "
import sys
try:
    import setuptools
    sys.exit(0)
except ImportError as e:
    print(f'{e}. Search paths:', file=sys.stderr)
    for p in sys.path: print(f'  {p}', file=sys.stderr)
    sys.exit(1)
"
    RESULT_VARIABLE has_setuptools)

    if(has_setuptools EQUAL "1")
        message(FATAL_ERROR "Python module `setuptools` required for correct wheel filename generation.")
    endif()

    execute_process(
        COMMAND ${Python3_EXECUTABLE} -c "
import sys
try:
    from wheel.bdist_wheel import bdist_wheel
    sys.exit(0)
except ImportError as e:
    print(f'{e}. Search paths:', file=sys.stderr)
    for p in sys.path: print(f'  {p}', file=sys.stderr)
    sys.exit(1)
"
    RESULT_VARIABLE has_bdist_wheel)

    if(has_bdist_wheel EQUAL "1")
        message(FATAL_ERROR "Python module `wheel.bdist_wheel` required for correct wheel filename generation.")
    endif()

    execute_process(
        COMMAND ${Python3_EXECUTABLE} -c "
from setuptools.dist import Distribution
from setuptools import Extension

def wheel_name(**kwargs):
    # create a fake distribution from arguments
    dist = Distribution(attrs=kwargs)
    # finalize bdist_wheel command
    bdist_wheel_cmd = dist.get_command_obj('bdist_wheel')
    bdist_wheel_cmd.ensure_finalized()
    # assemble wheel file name
    distname = bdist_wheel_cmd.wheel_dist_name
    tag = '-'.join(bdist_wheel_cmd.get_tag())
    return f'{distname}-{tag}'

print(wheel_name(name='${python_module}', version='${version}', ext_modules=[Extension('dummy', ['dummy.c'])]))
"
        OUTPUT_VARIABLE wheel_filename
        OUTPUT_STRIP_TRAILING_WHITESPACE
        RESULT_VARIABLE has_wheel_filename
    )

    if(NOT has_wheel_filename EQUAL "0")
        message(FATAL_ERROR "Cannot format wheel filename via 'setuptools'.")
    endif()

    set(wheel_filename "${CMAKE_BINARY_DIR}/${wheel_filename}.whl")
    set(wheel_distinfo "${CMAKE_BINARY_DIR}/${python_module}-${version}.dist-info")
    set(wheel_data "${CMAKE_BINARY_DIR}/${python_module}-${version}.data")
    set(wheel_generator_string "pango_wheelgen_${version}")

    if( MAKEWHEEL_REQUIRES )
        set(MAKEWHEEL_REQUIRES "Requires-Dist: ${MAKEWHEEL_REQUIRES}")
        string(REPLACE ";" "\nRequires-Dist: " MAKEWHEEL_REQUIRES "${MAKEWHEEL_REQUIRES}")
    endif()

    ##########################################
    ## Create dist info folder
    file(GLOB wheel_info_files "${_MAKE_PYTHON_WHEEL_LIST_DIR}/wheel-dist-info/*" )
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

    add_custom_target(
        "${python_module}_pip_install"
        DEPENDS "${python_module}_wheel"
        COMMAND ${Python3_EXECUTABLE} -mpip install "${wheel_filename}" --force-reinstall
        COMMENT "Installing for selected Python '${Python3_EXECUTABLE}'"
    )

    add_custom_target(
        "${python_module}_pip_uninstall"
        COMMAND ${Python3_EXECUTABLE} -mpip uninstall -y ${python_module}
    )

    ##########################################
    ## Help message since this is tricky for people
    if(MAKEWHEEL_PRINT_HELP)
        message(STATUS "Selected Python: '${Python3_EXECUTABLE}'. cmake --build . -t ${python_module}_pip_install to use ${python_module} module.")
    endif()
endfunction()


