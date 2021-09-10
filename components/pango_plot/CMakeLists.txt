get_filename_component(COMPONENT ${CMAKE_CURRENT_LIST_DIR} NAME)

target_sources( ${COMPONENT}
PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/src/datalog.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/plotter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/loaders/csv_table_loader.cpp
)

target_link_libraries(${COMPONENT} pango_display)
target_include_directories(${COMPONENT} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
install(DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/include"
  DESTINATION ${CMAKE_INSTALL_PREFIX}
)
