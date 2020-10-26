get_filename_component(COMPONENT ${CMAKE_CURRENT_LIST_DIR} NAME)

target_sources( ${COMPONENT}
PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/src/vars.cpp
)

target_include_directories(${COMPONENT} PUBLIC "${CMAKE_CURRENT_LIST_DIR}/include")
target_link_libraries(${COMPONENT} PUBLIC pango_core)
target_compile_definitions(${COMPONENT} PUBLIC BUILD_PANGOLIN_VARS)
