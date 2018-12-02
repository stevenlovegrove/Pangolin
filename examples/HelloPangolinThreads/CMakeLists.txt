# Find Pangolin (https://github.com/stevenlovegrove/Pangolin)
find_package(Pangolin 0.5 REQUIRED)
find_package(Threads QUIET)

if(Theads_FOUND)
    add_executable(HelloPangolinThreads main.cpp)
    target_include_directories(HelloPangolinThreads ${Pangolin_INCLUDE_DIRS})
    target_link_libraries(HelloPangolinThreads ${Pangolin_LIBRARIES} Threads::Threads)
endif()
