# Find Pangolin (https://github.com/stevenlovegrove/Pangolin)
find_package(Pangolin 0.4 REQUIRED)
include_directories(../../include ../../src/include ${CMAKE_BINARY_DIR}/src/include)


append_glob(LOG_SOURCES ../../src/log/*.cpp)
append_glob(LOG_SOURCES ../../src/utils/*.cpp)

add_executable(testlog testlog.cpp ${LOG_SOURCES})
target_link_libraries(testlog pthread)
