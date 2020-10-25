get_filename_component(COMPONENT ${CMAKE_CURRENT_LIST_DIR} NAME)

target_sources( ${COMPONENT}
PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/src/datalog.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/plotter.cpp
)

target_include_directories(${COMPONENT} PUBLIC "${CMAKE_CURRENT_LIST_DIR}/include")
target_link_libraries(${COMPONENT} pango_display)
