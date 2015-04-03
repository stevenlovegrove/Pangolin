# Find Pangolin (https://github.com/stevenlovegrove/Pangolin)
find_package(pangolin 0.1 REQUIRED)
include_directories(${PANGOLIN_INCLUDE_DIRS})

add_executable(SimpleRepeatVideo main.cpp)
target_link_libraries(SimpleRepeatVideo ${PANGOLIN_LIBRARIES})
