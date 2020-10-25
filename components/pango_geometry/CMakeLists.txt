get_filename_component(COMPONENT ${CMAKE_CURRENT_LIST_DIR} NAME)

target_sources( ${COMPONENT}
PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/src/geometry.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/geometry_obj.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/geometry_ply.cpp
)

target_link_libraries(${COMPONENT} pango_core pango_image tinyobj)
target_include_directories(${COMPONENT} PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include )
