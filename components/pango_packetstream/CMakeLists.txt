get_filename_component(COMPONENT ${CMAKE_CURRENT_LIST_DIR} NAME)

target_sources( ${COMPONENT}
PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/src/packet.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/packetstream.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/packetstream_reader.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/packetstream_writer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/playback_session.cpp
)

target_include_directories(${COMPONENT} PUBLIC "${CMAKE_CURRENT_LIST_DIR}/include")
target_compile_definitions(${COMPONENT} PRIVATE "PANGOLIN_VERSION_STRING=\"${PANGOLIN_VERSION}\"")
target_link_libraries(${COMPONENT} PUBLIC pango_core)
