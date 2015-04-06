# Find Pangolin (https://github.com/stevenlovegrove/Pangolin)
include_directories(${PANGOLIN_INCLUDE_DIRS})

add_executable(SimpleDisplayImage main.cpp)
target_link_libraries(SimpleDisplayImage ${PANGOLIN_LIBRARIES})
