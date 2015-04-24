# Find Pangolin (https://github.com/stevenlovegrove/Pangolin)
include_directories(${PANGOLIN_INCLUDE_DIRS})

add_executable(SimpleDisplay main.cpp)
target_link_libraries(SimpleDisplay ${PANGOLIN_LIBRARIES})
