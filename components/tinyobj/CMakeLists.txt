set(COMPONENT tinyobj)

target_sources( ${COMPONENT}
    PUBLIC  "${CMAKE_CURRENT_LIST_DIR}/include/tinyobj/tiny_obj_loader.h"
    PRIVATE "${CMAKE_CURRENT_LIST_DIR}/src/tinyobj.cpp"
)

target_include_directories(${COMPONENT} PUBLIC "${CMAKE_CURRENT_LIST_DIR}/include")
set_property(TARGET ${COMPONENT} PROPERTY CXX_STANDARD 11)
