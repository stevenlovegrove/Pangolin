get_filename_component(COMPONENT ${CMAKE_CURRENT_LIST_DIR} NAME)

set(component_file_basenames
  context
  conventions
  eigen_numeric_limits
  eigen_scalar_methods
  handler
  look_at
  min_max
  shared
  widget_panel
  gl_framebuffer_reader
  gl_render_panel
)

foreach(basename ${component_file_basenames})
  target_sources( ${COMPONENT} PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src/${basename}.cpp )

  if(BUILD_TESTS)
      add_executable(${basename}_test ${CMAKE_CURRENT_LIST_DIR}/src/${basename}_test.cpp)
      target_link_libraries(${basename}_test PRIVATE Catch2::Catch2 ${COMPONENT})
      catch_discover_tests(${basename}_test)
  endif()
endforeach()

target_link_libraries(${COMPONENT} PUBLIC pango_opengl pango_windowing pango_vars )

target_include_directories(${COMPONENT} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
install(DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/include"
  DESTINATION ${CMAKE_INSTALL_PREFIX}
)
