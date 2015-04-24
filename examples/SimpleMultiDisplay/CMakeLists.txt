# Find Pangolin (https://github.com/stevenlovegrove/Pangolin)
include_directories(${PANGOLIN_INCLUDE_DIRS})

add_executable(SimpleMultiDisplay main.cpp)
target_link_libraries(SimpleMultiDisplay ${PANGOLIN_LIBRARIES})
