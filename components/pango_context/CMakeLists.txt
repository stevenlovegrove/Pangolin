get_filename_component(COMPONENT ${CMAKE_CURRENT_LIST_DIR} NAME)

set(component_file_basenames
  context
  depth_sampler_image
  draw_layer_handler
  layer_group
  make_drawable
  gl_context/gl_depth_sampler
  gl_drawable/gl_drawn_checker
  gl_drawable/gl_drawn_group
  gl_drawable/gl_drawn_image
  gl_drawable/gl_drawn_plot_background
  gl_drawable/gl_drawn_primitives
  gl_drawable/gl_drawn_solids
  gl_drawable/gl_drawn_text
  gl_layer/gl_clear_layer
  gl_layer/gl_draw_layer
  gl_layer/gl_widget_layer
)

foreach(basename ${component_file_basenames})
  target_sources( ${COMPONENT} PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src/${basename}.cpp )
endforeach()

if(BUILD_TESTS)
  add_executable(layer_group_test ${CMAKE_CURRENT_LIST_DIR}/src/layer_group_test.cpp)
  target_link_libraries(layer_group_test PRIVATE Catch2::Catch2WithMain ${COMPONENT})
  catch_discover_tests(layer_group_test)
endif()

target_link_libraries(${COMPONENT} PUBLIC pango_windowing pango_render pango_vars farm_ng_core::sophus_image farm_ng_core::sophus_sensor)
target_link_libraries(${COMPONENT} PUBLIC pango_opengl)

find_package(assimp QUIET)
if(assimp_FOUND)
  message(STATUS "libassimp Found and Enabled")
  target_compile_definitions(${COMPONENT} PRIVATE HAVE_ASSIMP)
  target_link_libraries(${COMPONENT} PRIVATE assimp::assimp)
endif()

target_include_directories(${COMPONENT} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
install(DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/include"
  DESTINATION ${CMAKE_INSTALL_PREFIX}
)
