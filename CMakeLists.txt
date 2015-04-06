cmake_minimum_required(VERSION 2.8)
project( Pangolin )
set(PANGOLIN_VERSION_MAJOR 0)
set(PANGOLIN_VERSION_MINOR 2)
set(VERSION ${PANGOLIN_VERSION_MAJOR}.${PANGOLIN_VERSION_MINOR})

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules/")
include( install_library )

option( BUILD_SHARED_LIBS "Build Shared Library" ON)
option( BUILD_EXAMPLES "Build Examples" ON )
option( CPP11_NO_BOOST "Use c++11 over boost for threading etc." ON )

if(CPP11_NO_BOOST AND NOT MSVC)
    set( CMAKE_CXX_FLAGS "-std=c++11 -Wall ${CMAKE_CXX_FLAGS}" )
    if(_CLANG_)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
    endif()
endif()


string( TOLOWER ${PROJECT_NAME} LIBRARY_NAME )

if(ANDROID OR IOS)
    set(HAVE_GLES 1)
    option(BUILD_FOR_GLES_2 "Build for OpenGL ES 2 instead of ES 1" ON )
    if(BUILD_FOR_GLES_2)
        set(HAVE_GLES_2 1)
    endif()
endif()

# Overide with cmake -DCMAKE_BUILD_TYPE=Debug {dir}
if( NOT CMAKE_BUILD_TYPE )
   message("Build type not set (defaults to release)")
   message("-DCMAKE_BUILD_TYPE=Debug for debug")
   set( CMAKE_BUILD_TYPE Release )
endif()

# Platform configuration vars
include(SetPlatformVars)

# Sets PANGOLIN_SRCS
add_subdirectory( "src" )

message( STATUS "LIBRARY_NAME ${LIBRARY_NAME}" )

# Include collected includes/libraries
include_directories( ${REQUIRED_INCLUDE_DIRS} ${CMAKE_CURRENT_BINARY_DIR}/include )
include_directories( ${CMAKE_SOURCE_DIR}/include ${CMAKE_BINARY_DIR} )
add_library( pangolin ${PANGOLIN_SRCS} )
target_link_libraries( pangolin ${REQUIRED_LIBRARIES} )

install_library(
    LIBRARY pangolin
    VERSION ${VERSION}
    DESCRIPTION "GL wrapper library that permits simpler GUI creation"
    HEADERS ${PANGOLIN_HDRS}
    HEADER_DESTINATION ${CMAKE_INSTALL_PREFIX}/include/Pangolin
    INCLUDE_DIRS ${REQUIRED_INCLUDE_DIRS}
    LIB_DEPENDS ${REQUIRED_LIBRARIES}
    LINK_DIRS /usr/local/lib
    )


#######################################################
## Add Libraries / Include Directories / Link directories

## Generate symbol export helper header on MSVC
IF(MSVC)
    string(TOUPPER ${LIBRARY_NAME} LIBRARY_NAME_CAPS)
    include(GenerateExportHeader)
    GENERATE_EXPORT_HEADER( ${LIBRARY_NAME}
        BASE_NAME ${LIBRARY_NAME_CAPS}
        EXPORT_MACRO_NAME ${LIBRARY_NAME_CAPS}_EXPORT
        EXPORT_FILE_NAME "${CMAKE_CURRENT_BINARY_DIR}/include/${LIBRARY_NAME}/${LIBRARY_NAME}_export.h"
        STATIC_DEFINE ${LIBRARY_NAME_CAPS}_BUILT_AS_STATIC
    )
ENDIF()

## Set Special Compiler flags
if(MSVC)
  set(CMAKE_CXX_FLAGS "/EHs ${CMAKE_CXX_FLAGS}")
elseif(CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS "-Wall -Wno-error=deprecated-declarations ${CMAKE_CXX_FLAGS}")
endif()

#######################################################
## Generate Doxygen documentation target (make doc)
find_package(Doxygen)
if(DOXYGEN_FOUND)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
    file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/../doc )
    add_custom_target(doc
        ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/../doc
        COMMENT "Generating API documentation with Doxygen" VERBATIM
    )
endif()

#######################################################
## Install script

set( PANGOLIN_DIR ${CMAKE_CURRENT_BINARY_DIR} )
set( PANGOLIN_LIBRARIES ${REQUIRED_LIBRARIES} pangolin )
set( PANGOLIN_INCLUDE_DIRS ${REQUIRED_INCLUDE_DIRS} )
link_libraries( ${REQUIRED_LIBRARIES} pangolin )

if(BUILD_EXAMPLES)
  add_subdirectory(examples)
endif()

# make an uninstall target
include(${CMAKE_MODULE_PATH}/cmake_uninstall.cmake.in)
add_custom_target(uninstall
    COMMAND "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")
