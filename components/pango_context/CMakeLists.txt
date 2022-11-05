get_filename_component(COMPONENT ${CMAKE_CURRENT_LIST_DIR} NAME)

set(component_file_basenames
  context
  conventions
  handler
  look_at
  projection
  min_max
  shared
  gl_widget_layer
  depth_sampler
  gl_draw_layer
  gl_device_buffer
  layer_group
  drawable
  multi_dim
)

foreach(basename ${component_file_basenames})
  target_sources( ${COMPONENT} PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src/${basename}.cpp )

  if(BUILD_TESTS)
      add_executable(${basename}_test ${CMAKE_CURRENT_LIST_DIR}/src/${basename}_test.cpp)
      target_link_libraries(${basename}_test PRIVATE Catch2::Catch2WithMain ${COMPONENT})
      catch_discover_tests(${basename}_test)
  endif()
endforeach()

find_package(Sophus REQUIRED)
target_link_libraries(${COMPONENT} PUBLIC pango_opengl pango_windowing pango_vars Sophus::sophus_image Sophus::sophus_sensor)

target_include_directories(${COMPONENT} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
install(DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/include"
  DESTINATION ${CMAKE_INSTALL_PREFIX}
)
