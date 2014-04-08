# Find Pangolin (https://github.com/stevenlovegrove/Pangolin)
find_package(Pangolin 0.1 REQUIRED)
include_directories(${Pangolin_INCLUDE_DIRS})

add_executable(OculusDemo main.cpp OculusHud.cpp)
target_link_libraries(OculusDemo ${Pangolin_LIBRARIES})
