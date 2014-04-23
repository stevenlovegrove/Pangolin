cmake_minimum_required(VERSION 2.6)
project("Pangolin")
set(PANGOLIN_VERSION_MAJOR 0)
set(PANGOLIN_VERSION_MINOR 1)
set(PANGOLIN_VERSION ${PANGOLIN_VERSION_MAJOR}.${PANGOLIN_VERSION_MINOR})

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules/")

option( BUILD_SHARED_LIBS "Build Shared Library" ON)
option( BUILD_EXAMPLES "Build Examples" ON )
option( CPP11_NO_BOOST "Use c++11 over boost for threading etc." OFF )

if(CPP11_NO_BOOST)
    set( CMAKE_CXX_FLAGS "-std=c++0x -Wall ${CMAKE_CXX_FLAGS}" )
    if(_CLANG_)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
    endif()
endif()

if(ANDROID)
   set(ANDROID_PACKAGE_NAME "com.github.stevenlovegrove.pangolin")
   include(AndroidUtils)
endif()

# Overide with cmake -DCMAKE_BUILD_TYPE=Debug {dir}
if( NOT CMAKE_BUILD_TYPE )
   message("Build type not set (defaults to release)")
   message("-DCMAKE_BUILD_TYPE=Debug for debug")
   set( CMAKE_BUILD_TYPE Release )
endif()

# Platform configuration vars
include(SetPlatformVars)

string(TOLOWER ${PROJECT_NAME} LIBRARY_NAME)

# make an uninstall target 
configure_file(
 	"${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
  	"${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
  	IMMEDIATE @ONLY
)

add_custom_target(uninstall
  "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")

add_subdirectory("src")

if(BUILD_EXAMPLES)
  set(Pangolin_DIR ${Pangolin_BINARY_DIR}/src)
  add_subdirectory(examples)
endif()
