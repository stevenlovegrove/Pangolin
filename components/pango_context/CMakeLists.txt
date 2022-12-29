get_filename_component(COMPONENT ${CMAKE_CURRENT_LIST_DIR} NAME)

set(component_file_basenames
  context
  conventions
  draw_layer_handler
  look_at
  projection
  min_max
  shared
  depth_sampler
  depth_sampler_image
  make_drawable
  gl_widget_layer
  gl_draw_layer
  gl_device_buffer
  gl_device_texture
  gl_vao
  gl_drawn_image
  gl_drawn_primitives
  gl_drawn_solids
  gl_drawn_checker
  gl_drawn_plot_background
  gl_drawn_text
  layer
  layer_group
  multi_dim
)

foreach(basename ${component_file_basenames})
  target_sources( ${COMPONENT} PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src/${basename}.cpp )

  if(PANGOLIN_BUILD_TESTS)
      add_executable(${basename}_test ${CMAKE_CURRENT_LIST_DIR}/src/${basename}_test.cpp)
      target_link_libraries(${basename}_test PRIVATE Catch2::Catch2WithMain ${COMPONENT})
      catch_discover_tests(${basename}_test)
  endif()
endforeach()

find_package(Sophus REQUIRED)
find_package(fmt REQUIRED)
target_link_libraries(${COMPONENT} PUBLIC pango_opengl pango_windowing pango_vars sophus_image sophus_sensor fmt::fmt)

target_include_directories(${COMPONENT} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
install(DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/include"
  DESTINATION ${CMAKE_INSTALL_PREFIX}
)
