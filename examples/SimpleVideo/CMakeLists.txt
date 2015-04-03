# Find Pangolin (https://github.com/stevenlovegrove/Pangolin)
find_package(pangolin 0.1 REQUIRED)
include_directories(${PANGOLIN_INCLUDE_DIRS})

add_executable(SimpleVideo main.cpp)
target_link_libraries(SimpleVideo ${PANGOLIN_LIBRARIES})
