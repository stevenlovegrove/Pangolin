get_filename_component(COMPONENT ${CMAKE_CURRENT_LIST_DIR} NAME)

option(BUILD_PANGOLIN_PYTHON "Build support for Pangolin Interactive Console" ON)

if(BUILD_PANGOLIN_PYTHON)
  if(POLICY CMP0148)
    cmake_policy(SET CMP0148 NEW)
  endif()
  find_package(Python3 COMPONENTS Interpreter Development REQUIRED)
endif()

if(BUILD_PANGOLIN_PYTHON AND Python3_FOUND)
    # If we've inited the submodule, use that
    if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/pybind11/CMakeLists.txt")
        add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/pybind11)
    else()
        find_package(pybind11 CONFIG QUIET)
    endif()
endif()

if(BUILD_PANGOLIN_PYTHON AND Python3_FOUND AND pybind11_FOUND)
    set(SRC_BINDINGS
        ${CMAKE_CURRENT_LIST_DIR}/src/pypangolin/attach.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/pypangolin/colour.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/pypangolin/datalog.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/pypangolin/display.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/pypangolin/gl.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/pypangolin/gl_draw.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/pypangolin/gltext.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/pypangolin/glfont.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/pypangolin/glsl.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/pypangolin/glvbo.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/pypangolin/handler.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/pypangolin/image.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/pypangolin/image_view.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/pypangolin/opengl_render_state.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/pypangolin/params.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/pypangolin/pixel_format.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/pypangolin/plotter.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/pypangolin/pypangoio.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/pypangolin/var.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/pypangolin/video.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/pypangolin/view.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/pypangolin/viewport.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/pypangolin/widget.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/pypangolin/window.cpp
    )

    target_sources(
        ${COMPONENT} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/src/pyinterpreter.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/pypangolin_embed.cpp
        ${SRC_BINDINGS}
    )

    target_compile_definitions(${COMPONENT} PUBLIC HAVE_PYTHON)
    target_link_libraries(${COMPONENT} PUBLIC
        pango_core pango_display pango_plot pango_video pango_vars
    )

    set_target_properties(
        ${COMPONENT} PROPERTIES VERSION ${PANGOLIN_VERSION} SOVERSION ${PANGOLIN_VERSION_MAJOR}
    )

    target_link_libraries(${COMPONENT} PRIVATE pybind11::embed )
    target_include_directories(${COMPONENT} PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
        $<INSTALL_INTERFACE:include>
    )
    install(DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/include"
        DESTINATION ${CMAKE_INSTALL_PREFIX}
    )

    if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_C_COMPILER_ID MATCHES "Clang")
        # ignore "maybe-uninitialized" and "maybe-uninitialized" for Eigen
        if(CMAKE_COMPILER_IS_GNUCC)
            target_compile_options(${COMPONENT} PRIVATE  "-Wno-error=maybe-uninitialized")
        endif()
        target_compile_options(${COMPONENT} PRIVATE  "-Wno-error=uninitialized")
    endif()

    PangolinRegisterFactory( InterpreterInterface PyInterpreter )

    # Add python module as another target. For some reason, it doesn't work to just link against ${COMPONENT}
    pybind11_add_module(pypangolin ${CMAKE_CURRENT_LIST_DIR}/src/pypangolin_module.cpp ${SRC_BINDINGS})
    target_link_libraries(pypangolin PRIVATE pango_core pango_display pango_plot pango_video pango_vars pybind11::module)
    target_include_directories(pypangolin PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
        $<INSTALL_INTERFACE:include>
    )

    # To prevent some visibility warnings.
    # https://pybind11.readthedocs.io/en/stable/faq.html#someclass-declared-with-greater-visibility-than-the-type-of-its-field-someclass-member-wattributes
    set_target_properties(pypangolin PROPERTIES CXX_VISIBILITY_PRESET hidden)
    set_target_properties(${COMPONENT} PROPERTIES CXX_VISIBILITY_PRESET hidden)

    # Create a target for generating a python wheel
    include(MakePythonWheel)
    MakeWheel( pypangolin
        VERSION ${PANGOLIN_VERSION}
        SUMMARY "Pangolin is a lightweight portable rapid development library for managing OpenGL display / interaction and abstracting video input."
        HOMEPAGE "https://github.com/stevenlovegrove/Pangolin"
        AUTHOR "Steven Lovegrove"
        EMAIL "stevenlovegrove@gmail.com"
        LICENCE "MIT"
        REQUIRES "pyopengl;numpy;pillow;pybind11"
        PRINT_HELP
    )
else()
    #TODO: with the following line uncommented and BUILD_PANGOLIN_PYTHON=OFF, `make install` fails b/c it tries to install a missing libpango_python.a
    #set_target_properties(${COMPONENT} PROPERTIES EXCLUDE_FROM_ALL 1 EXCLUDE_FROM_DEFAULT_BUILD 1)
    target_sources( ${COMPONENT} PRIVATE ${CMAKE_CURRENT_LIST_DIR}/../pango_core/src/dummy.cpp )
endif()
