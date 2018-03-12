# Find Pangolin (https://github.com/stevenlovegrove/Pangolin)
find_package(Pangolin 0.4 REQUIRED)
include_directories(${Pangolin_INCLUDE_DIRS})

if(EMSCRIPTEN)
    set ( CMAKE_CXX_FLAGS "--embed-file ${CMAKE_CURRENT_SOURCE_DIR}/app.cfg@app.cfg ${CMAKE_CXX_FLAGS}" )
endif()

add_executable(SimpleDisplay main.cpp)
target_link_libraries(SimpleDisplay ${Pangolin_LIBRARIES} )
