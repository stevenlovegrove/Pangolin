# Find Pangolin (https://github.com/stevenlovegrove/Pangolin)
find_package(Pangolin 0.8 REQUIRED)

find_package(Threads QUIET)

if(Threads_FOUND)
    include_directories(${Pangolin_INCLUDE_DIRS})
    add_executable(HelloPangolinThreads main.cpp)
    target_link_libraries(HelloPangolinThreads pango_display Threads::Threads)
endif()
