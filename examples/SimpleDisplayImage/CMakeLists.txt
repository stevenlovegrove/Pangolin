# Find Pangolin (https://github.com/stevenlovegrove/Pangolin)
find_package(Pangolin 0.2 REQUIRED)
include_directories(${Pangolin_INCLUDE_DIRS})

add_executable(SimpleDisplayImage main.cpp)
target_link_libraries(SimpleDisplayImage ${Pangolin_LIBRARIES})
