cmake_minimum_required(VERSION 3.8)

project("Pangolin")
set(PANGOLIN_VERSION_MAJOR 0)
set(PANGOLIN_VERSION_MINOR 5)
set(PANGOLIN_VERSION ${PANGOLIN_VERSION_MAJOR}.${PANGOLIN_VERSION_MINOR})

# Prevent in source builds
if(EXISTS "${PROJECT_BINARY_DIR}/CMakeLists.txt")
    message(FATAL_ERROR "Source build detected: please use a subdir. You may remove 'CMakeCache.txt' and 'CMakeFiles'.")
endif()

# Make our own cmake imports accessible
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake" ${CMAKE_MODULE_PATH})

# Default build type (Override with cmake .. -DCMAKE_BUILD_TYPE=...)
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build." FORCE)
  message(STATUS "Setting build type to '${CMAKE_BUILD_TYPE}' as none was specified.")
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

include(CreateMethodCallFile)

option( BUILD_SHARED_LIBS "Build Shared Library" ON)

if(EMSCRIPTEN)
    include(EmscriptenUtils)
    set( CMAKE_CXX_FLAGS "-s EMTERPRETIFY=1 -s EMTERPRETIFY_ASYNC=1 -s DISABLE_EXCEPTION_CATCHING=0 -s ERROR_ON_UNDEFINED_SYMBOLS=0 -s USE_WEBGL2=1 -s FULL_ES2=1 -s FULL_ES3=1 ${CMAKE_CXX_FLAGS}" )
    set( CMAKE_C_FLAGS   "-s EMTERPRETIFY=1 -s EMTERPRETIFY_ASYNC=1 -s DISABLE_EXCEPTION_CATCHING=0 -s ERROR_ON_UNDEFINED_SYMBOLS=0 -s USE_WEBGL2=1 -s FULL_ES2=1 -s FULL_ES3=1 ${CMAKE_C_FLAGS}" )
endif()

# Add all pangolin components
file(GLOB components_cmake "${CMAKE_CURRENT_LIST_DIR}/components/*/CMakeLists.txt")
set(component_list "")
foreach(component_cmake ${components_cmake})
    get_filename_component(component_dir ${component_cmake} DIRECTORY)
    get_filename_component(component_name ${component_dir} NAME)
    add_library(${component_name} "")
    list(APPEND component_list ${component_name})
endforeach()
foreach(component_cmake ${components_cmake})
    include(${component_cmake})
endforeach()

# Export Targets
#export(TARGETS ${component_list} NAMESPACE pangolin:: FILE PangolinTargets.cmake)
export(TARGETS ${component_list} FILE PangolinTargets.cmake)
export(PACKAGE Pangolin)

# Version information
configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/cmake/${PROJECT_NAME}ConfigVersion.cmake.in
  "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake" @ONLY)

# Build tree config
configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/cmake/${PROJECT_NAME}Config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake @ONLY IMMEDIATE )

set(Pangolin_DIR ${CMAKE_CURRENT_BINARY_DIR})
add_subdirectory(examples)
add_subdirectory(tools)
