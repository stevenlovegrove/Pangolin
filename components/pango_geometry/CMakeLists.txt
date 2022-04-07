get_filename_component(COMPONENT ${CMAKE_CURRENT_LIST_DIR} NAME)

find_package (Eigen3 REQUIRED QUIET)
message(STATUS "Found Eigen: '${EIGEN3_INCLUDE_DIRS}'")
target_compile_definitions(${COMPONENT} PUBLIC HAVE_EIGEN)

target_sources( ${COMPONENT}
PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/src/geometry.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/geometry_obj.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/geometry_ply.cpp
)

target_link_libraries(${COMPONENT} pango_core pango_image tinyobj Eigen3::Eigen)
target_include_directories(${COMPONENT} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
install(DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/include"
  DESTINATION ${CMAKE_INSTALL_PREFIX}
)
