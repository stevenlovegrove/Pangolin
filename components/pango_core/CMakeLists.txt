get_filename_component(COMPONENT ${CMAKE_CURRENT_LIST_DIR} NAME)

target_sources( ${COMPONENT}
PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/src/file_extension.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/file_utils.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/sigstate.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/threadedfilebuf.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/uri.cpp
)

target_include_directories(${COMPONENT} PUBLIC "${CMAKE_CURRENT_LIST_DIR}/include")
target_compile_features(${COMPONENT} PUBLIC cxx_decltype_auto )
