get_filename_component(COMPONENT ${CMAKE_CURRENT_LIST_DIR} NAME)

target_sources( ${COMPONENT}
PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/src/glgeometry.cpp
)

set_target_properties(
    ${COMPONENT} PROPERTIES VERSION ${PANGOLIN_VERSION} SOVERSION ${PANGOLIN_VERSION_MAJOR}
)

target_link_libraries(${COMPONENT} pango_geometry pango_opengl)
target_include_directories(${COMPONENT} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
install(DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/include"
  DESTINATION ${CMAKE_INSTALL_PREFIX}
)
