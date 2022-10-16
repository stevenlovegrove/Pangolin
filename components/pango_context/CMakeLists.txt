get_filename_component(COMPONENT ${CMAKE_CURRENT_LIST_DIR} NAME)

target_sources( ${COMPONENT}
PRIVATE
${CMAKE_CURRENT_LIST_DIR}/src/context.cpp
${CMAKE_CURRENT_LIST_DIR}/src/widgets.cpp
)

target_link_libraries(${COMPONENT} PUBLIC pango_core pango_opengl pango_windowing pango_vars )

target_include_directories(${COMPONENT} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
install(DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/include"
  DESTINATION ${CMAKE_INSTALL_PREFIX}
)

if(BUILD_TESTS)
    add_executable(test_shared ${CMAKE_CURRENT_LIST_DIR}/tests/test_shared.cpp)
    target_link_libraries(test_shared PRIVATE Catch2::Catch2 ${COMPONENT})
    catch_discover_tests(test_shared)
endif()