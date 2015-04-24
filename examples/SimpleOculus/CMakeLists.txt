# Find Pangolin (https://github.com/stevenlovegrove/Pangolin)
include_directories(${PANGOLIN_INCLUDE_DIRS})

add_executable(SimpleOculus main.cpp)
target_link_libraries(SimpleOculus ${PANGOLIN_LIBRARIES})
