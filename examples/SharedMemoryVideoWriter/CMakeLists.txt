if(UNIX)
find_package(Pangolin 0.2 REQUIRED)
include_directories(${Pangolin_INCLUDE_DIRS})

add_executable(SharedMemoryVideoWriter main.cpp)
target_link_libraries(SharedMemoryVideoWriter ${Pangolin_LIBRARIES})
endif()
