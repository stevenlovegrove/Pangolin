get_filename_component(COMPONENT ${CMAKE_CURRENT_LIST_DIR} NAME)

include(EmbedBinaryFiles)
file(GLOB_RECURSE BINARY_FILES_TO_EMBED "${CMAKE_CURRENT_LIST_DIR}/assets/*.*")
embed_binary_files_rule(assets.cpp "${BINARY_FILES_TO_EMBED}")

target_sources( ${COMPONENT} PRIVATE
  ${CMAKE_CURRENT_BINARY_DIR}/assets.cpp
  ${CMAKE_CURRENT_LIST_DIR}/src/mascot.cpp )

target_link_libraries(${COMPONENT} PUBLIC pango_context)

target_include_directories(${COMPONENT} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
install(DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/include"
  DESTINATION ${CMAKE_INSTALL_PREFIX}
)
