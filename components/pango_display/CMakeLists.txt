get_filename_component(COMPONENT ${CMAKE_CURRENT_LIST_DIR} NAME)

target_sources( ${COMPONENT}
PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/src/display.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/process.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/pangolin_gl.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/handler.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/handler_image.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/handler_glbuffer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/view.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/widgets.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/image_view.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/ConsoleView.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/default_font.cpp
)

target_link_libraries(${COMPONENT} PUBLIC pango_core pango_opengl pango_windowing pango_vars )

target_include_directories(${COMPONENT} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
install(DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/include"
  DESTINATION ${CMAKE_INSTALL_PREFIX}
)
