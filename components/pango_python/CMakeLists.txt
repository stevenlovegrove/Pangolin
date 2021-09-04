get_filename_component(COMPONENT ${CMAKE_CURRENT_LIST_DIR} NAME)

find_package(Python COMPONENTS Interpreter Development QUIET)
if(Python_FOUND)
    # If we've inited the submodule, use that
    if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/pybind11/CMakeLists.txt")
        add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/pybind11)
    else()
        find_package(pybind11 CONFIG QUIET)
    endif()
endif()

if(Python_FOUND AND pybind11_FOUND)
    set(SRC_BINDINGS
        ${CMAKE_CURRENT_LIST_DIR}/src/pypangolin/attach.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/pypangolin/colour.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/pypangolin/datalog.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/pypangolin/display.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/pypangolin/gl.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/pypangolin/gl_draw.cpp
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
    target_link_libraries(${COMPONENT}
        pango_core pango_display pango_plot pango_video pango_vars
        pybind11::embed 
    )
    target_include_directories(${COMPONENT} PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
        $<INSTALL_INTERFACE:include>
    )
    install(DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/include"
        DESTINATION ${CMAKE_INSTALL_PREFIX}
    )

    PangolinRegisterFactory( InterpreterInterface PyInterpreter )

    # Add python module as another target. For some reason, it doesn't work to just link against ${COMPONENT}
    pybind11_add_module(pypangolin ${CMAKE_CURRENT_LIST_DIR}/src/pypangolin_module.cpp ${SRC_BINDINGS})
    target_link_libraries(pypangolin PRIVATE pango_core pango_display pango_plot pango_video pango_vars pybind11::module)
    target_include_directories(pypangolin PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
        $<INSTALL_INTERFACE:include>
    )
else()
    set_target_properties(${COMPONENT} PROPERTIES EXCLUDE_FROM_ALL 1 EXCLUDE_FROM_DEFAULT_BUILD 1)
    target_sources( ${COMPONENT} PRIVATE ${CMAKE_CURRENT_LIST_DIR}/../pango_core/src/dummy.cpp )
endif()
