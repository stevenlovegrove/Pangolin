get_filename_component(COMPONENT ${CMAKE_CURRENT_LIST_DIR} NAME)

target_sources( ${COMPONENT}
PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/src/PacketLog.cpp
)

target_include_directories(${COMPONENT}
    PUBLIC "${CMAKE_CURRENT_LIST_DIR}/include"
    "${CMAKE_CURRENT_LIST_DIR}/../../third_party/stduuid/include"
    "${CMAKE_CURRENT_LIST_DIR}/../../third_party/mio/include"
)
target_compile_definitions(${COMPONENT} PRIVATE "PANGOLIN_VERSION_STRING=\"${PANGOLIN_VERSION}\"")
target_link_libraries(${COMPONENT} PUBLIC pango_core)
