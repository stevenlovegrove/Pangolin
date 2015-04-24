# Find Pangolin (https://github.com/stevenlovegrove/Pangolin)
include_directories(${PANGOLIN_INCLUDE_DIRS})

add_executable(SimplePlot main.cpp)
target_link_libraries(SimplePlot ${PANGOLIN_LIBRARIES})
