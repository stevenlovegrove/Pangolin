get_filename_component(COMPONENT ${CMAKE_CURRENT_LIST_DIR} NAME)

find_package(pybind11 CONFIG QUIET)

if(pybind11_FOUND)
    target_sources( ${COMPONENT}
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/src/pyinterpreter.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/pypangolin_init.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/pypangolin_module.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/pypangolin/attach.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/pypangolin/colour.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/pypangolin/datalog.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/pypangolin/display.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/pypangolin/gl_draw.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/pypangolin/handler.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/pypangolin/opengl_render_state.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/pypangolin/params.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/pypangolin/pixel_format.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/pypangolin/plotter.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/pypangolin/var.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/pypangolin/video.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/pypangolin/view.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/pypangolin/viewport.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/pypangolin/widget.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/pypangolin/window.cpp
    )

    target_include_directories(${COMPONENT}
        PUBLIC "${CMAKE_CURRENT_LIST_DIR}/include"
        PRIVATE ${PYTHON_INCLUDE_DIRS}
    )
    target_link_libraries(${COMPONENT}
        PRIVATE pango_core pango_display pango_plot pango_video pango_console pango_vars
        pybind11::pybind11 pybind11::embed pybind11::module
    )
    target_compile_definitions(${COMPONENT} PRIVATE HAVE_PYTHON)

else()
    set_target_properties(${COMPONENT} PROPERTIES EXCLUDE_FROM_ALL 1 EXCLUDE_FROM_DEFAULT_BUILD 1)
    target_sources( ${COMPONENT} PRIVATE ${CMAKE_CURRENT_LIST_DIR}/../pango_core/src/dummy.cpp )
endif()
