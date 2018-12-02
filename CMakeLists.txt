cmake_minimum_required(VERSION 3.8)
project("Pangolin")
set(PANGOLIN_VERSION_MAJOR 0)
set(PANGOLIN_VERSION_MINOR 5)
set(PANGOLIN_VERSION ${PANGOLIN_VERSION_MAJOR}.${PANGOLIN_VERSION_MINOR})

## Hunter is used to manage dependent libraries
## To control these manually, use cmake -DHUNTER_ENABLED=NO
#include("CMakeModules/HunterGate.cmake")
#HunterGate(
#    URL "https://github.com/ruslo/hunter/archive/v0.23.58.tar.gz"
#    SHA1 "ee80dc91f91f5eb6892f80a0a92c83b4de8e980a"
#)

project(Pangolin)

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


#hunter_add_package(glew)
#hunter_add_package(PNG)
#hunter_add_package(Jpeg)
#hunter_add_package(TIFF)
#hunter_add_package(lz4)
#hunter_add_package(pybind11)
#hunter_add_package(Eigen)

#find_package(glew REQUIRED)
#find_package(PNG)
#find_package(JPEG)
#find_package(TIFF)
#find_package(lz4)
#find_package(pybind11)
#find_package(Eigen3)

# TODO: Add EXR

#add_executable(foo test.cpp)
#target_link_libraries(foo glew::glew pybind11::pybind11 pybind11::embed pybind11::module PNG::png JPEG::jpeg lz4::lz4 TIFF::libtiff Eigen3::Eigen)

option( BUILD_SHARED_LIBS "Build Shared Library" ON)

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


export(TARGETS ${component_list} NAMESPACE pangolin:: FILE PangolinTargets.cmake)
#export(PACKAGE Pangolin)
