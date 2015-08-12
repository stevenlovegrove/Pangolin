# Find Pangolin (https://github.com/stevenlovegrove/Pangolin)
find_package(Pangolin 0.2 REQUIRED)
include_directories(${Pangolin_INCLUDE_DIRS})

find_package(PythonLibs REQUIRED)
include_directories(${PYTHON_INCLUDE_DIR})

add_executable(SimpleDisplay main.cpp)
target_link_libraries(SimpleDisplay ${Pangolin_LIBRARIES} ${PYTHON_LIBRARY})

