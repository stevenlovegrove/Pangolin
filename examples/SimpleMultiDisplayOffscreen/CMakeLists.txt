# Find Pangolin (https://github.com/stevenlovegrove/Pangolin)
find_package(Pangolin 0.5 REQUIRED)
include_directories(${Pangolin_INCLUDE_DIRS})

add_executable(SimpleMultiDisplayOffscreen main.cpp)
target_link_libraries(SimpleMultiDisplayOffscreen ${Pangolin_LIBRARIES})
