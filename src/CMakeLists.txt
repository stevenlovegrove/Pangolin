#######################################################
## Library sources

set( INCDIR "../include/pangolin")

set(
  HEADERS
  ${INCDIR}/pangolin.h
  ${INCDIR}/platform.h
  ${INCDIR}/simple_math.h
  ${INCDIR}/plotter.h 
  ${INCDIR}/display.h ${INCDIR}/display_internal.h 
  ${INCDIR}/view.h ${INCDIR}/viewport.h
  ${INCDIR}/handler.h ${INCDIR}/opengl_render_state.h
  ${INCDIR}/gl.h ${INCDIR}/gldraw.h ${INCDIR}/glvbo.h
  ${INCDIR}/glcuda.h ${INCDIR}/cg.h ${INCDIR}/glsl.h
  ${INCDIR}/threadedfilebuf.h ${INCDIR}/timer.h
)

set(
  SOURCES
  display.cpp view.cpp viewport.cpp
  handler.cpp opengl_render_state.cpp
  plotter.cpp
  threadedfilebuf.cpp
)

#######################################################
## User build options

option(BUILD_PANGOLIN_VARS "Build support for Pangolin Vars" ON)
if(BUILD_PANGOLIN_VARS)
  list(APPEND HEADERS ${INCDIR}/vars.h ${INCDIR}/vars_internal.h ${INCDIR}/input_record_repeat.h ${INCDIR}/widgets.h )
  list(APPEND SOURCES vars.cpp input_record_repeat.cpp widgets.cpp )
endif()

option(BUILD_PANGOLIN_VIDEO "Build support for Pangolin Video Utilities" ON)
if(BUILD_PANGOLIN_VIDEO)
  list(APPEND HEADERS ${INCDIR}/video.h ${INCDIR}/video_recorder.h ${INCDIR}/video_record_repeat.h ${INCDIR}/video/pvn_video.h )
  list(APPEND SOURCES video.cpp video_recorder.cpp video_record_repeat.cpp video/pvn_video.cpp )
endif()

#######################################################
## Find required dependencies

find_package(OpenGL REQUIRED QUIET)
find_package(GLEW REQUIRED)
find_package(Boost COMPONENTS thread filesystem system REQUIRED QUIET)

## Apply required dependencies settings
list(APPEND USER_INC  "${PROJECT_SOURCE_DIR}/include;${CMAKE_CURRENT_BINARY_DIR}/include" )
list(APPEND USER_INC  "${OPENGL_INCLUDE_DIR};${GLEW_INCLUDE_DIR};${Boost_INCLUDE_DIRS}" )
list(APPEND LINK_LIBS "${OPENGL_LIBRARIES};${GLEW_LIBRARY};${Boost_LIBRARIES}" )
list(APPEND LIB_DIRS  "${OPENGL_LIBRARY_DIR};${GLEW_LIBRARY_DIR};${Boost_LIBRARY_DIRS}" )

#######################################################
## Find optional dependencies

find_package(CVARS QUIET)
if(CVARS_FOUND)
  set(HAVE_CVARS 1)
  list(APPEND USER_INC  ${CVARS_INCLUDE_DIR} )
  list(APPEND LINK_LIBS ${CVARS_LIBRARIES} )
  list(APPEND LIB_DIRS ${CVARS_DIR} )
endif()

find_package(CUDA QUIET)
if(CUDA_FOUND)
  set(HAVE_CUDA 1)
  cuda_include_directories(${CMAKE_CURRENT_SOURCE_DIR})
  set(CUDA_ATTACH_VS_BUILD_RULE_TO_CUDA_FILE OFF)
  set(CUDA_NVCC_FLAGS "-O3" )
  add_definitions(-DCUDA_BUILD)
  message(STATUS "CUDA Found and Enabled")
endif()

find_package(FREEGLUT)
find_package(GLUT)

if(_OSX_)
  INCLUDE(CheckCXXSourceRuns)
  set(CMAKE_REQUIRED_LIBRARIES ${GLUT_LIBRARY})
  CHECK_CXX_SOURCE_RUNS("#include <GLUT/glut.h>\n int main () {return glutGetProcAddress(\"glutScrollFunc\") ? 0 : -1;};" HAVE_MODIFIED_OSXGLUT)
  if(NOT HAVE_MODIFIED_OSXGLUT)
        message(STATUS "Install modified osxglut for smooth scroll support / pinch / zoom.")
        message(STATUS "(https://github.com/stevenlovegrove/osxglut)")
  endif()
endif()

# Prefer OSX_MODIFIED_GLUT > FREEGLUT > GLUT
if(FREEGLUT_FOUND AND NOT HAVE_MODIFIED_OSXGLUT)
  set(HAVE_FREEGLUT 1)
  list(APPEND USER_INC  ${FREEGLUT_INCLUDE_DIR} )
  list(APPEND LINK_LIBS ${FREEGLUT_LIBRARY} )
elseif(GLUT_FOUND)
  list(APPEND USER_INC  ${GLUT_INCLUDE_DIR} )
  list(APPEND LINK_LIBS ${GLUT_LIBRARY} )
  if(_OSX_)
    set(HAVE_APPLE_OPENGL_FRAMEWORK 1)
    if(NOT HAVE_MODIFIED_OSXGLUT)
        message(STATUS "Using Apple Framework GLUT.")
        message(STATUS "Install Freeglut or modified osxglut for scroll support.")
    endif()
  endif()
endif()

if(FREEGLUT_FOUND OR GLUT_FOUND)
    set(HAVE_GLUT 1)
    message(STATUS "Glut Found and Enabled")
endif()

find_package(Eigen3 QUIET)
if(EIGEN3_FOUND)
  set(HAVE_EIGEN 1)
  list(APPEND USER_INC  ${EIGEN3_INCLUDE_DIR} )
  message(STATUS "Eigen Found and Enabled")
endif()

find_package(TooN QUIET)
if(TooN_FOUND)
  set(HAVE_TOON 1)
  list(APPEND USER_INC  ${TooN_INCLUDE_DIR} )
  message(STATUS "TooN Found and Enabled")
endif()

find_package(DC1394 QUIET)
if(BUILD_PANGOLIN_VIDEO AND DC1394_FOUND)
  set(HAVE_DC1394 1)
  list(APPEND INTERNAL_INC  ${DC1394_INCLUDE_DIR} )
  list(APPEND LINK_LIBS  ${DC1394_LIBRARY} )
  list(APPEND HEADERS ${INCDIR}/video/firewire.h )
  list(APPEND SOURCES video/firewire.cpp)
  message(STATUS "libdc1394 Found and Enabled")
endif()

if(BUILD_PANGOLIN_VIDEO AND _LINUX_)
  set(HAVE_V4L 1)
  list(APPEND HEADERS ${INCDIR}/video/v4l.h)
  list(APPEND SOURCES video/v4l.cpp)
endif()

find_package(FFMPEG QUIET)
if(BUILD_PANGOLIN_VIDEO AND FFMPEG_FOUND)
  set(HAVE_FFMPEG 1)
  list(APPEND INTERNAL_INC  ${FFMPEG_INCLUDE_DIRS} )
  list(APPEND LINK_LIBS ${FFMPEG_LIBRARIES} )
  list(APPEND HEADERS ${INCDIR}/video/ffmpeg.h)
  list(APPEND SOURCES video/ffmpeg.cpp)
  message(STATUS "ffmpeg Found and Enabled")
endif()

find_package(OpenNI QUIET)
if(BUILD_PANGOLIN_VIDEO AND OPENNI_FOUND)
  set(HAVE_OPENNI 1)
  list(APPEND INTERNAL_INC  ${OPENNI_INCLUDE_DIRS} )
  list(APPEND LINK_LIBS ${OPENNI_LIBRARIES} )
  list(APPEND HEADERS ${INCDIR}/video/openni.h )
  list(APPEND SOURCES video/openni.cpp)
  message(STATUS "OpenNI Found and Enabled")
endif()

# Generic Image Library (GIL) should come with boost.
# If you have a version of boost without it, maybe figure out how to test here.
set(HAVE_BOOST_GIL 1)

find_package(PNG QUIET)
if(PNG_FOUND)
  set(HAVE_PNG 1)
  list(APPEND INTERNAL_INC ${PNG_INCLUDE_DIR} )
  list(APPEND LINK_LIBS ${PNG_LIBRARY} )
endif()

find_package(JPEG QUIET)
if(JPEG_FOUND)
  set(HAVE_JPEG 1)
  list(APPEND INTERNAL_INC ${JPEG_INCLUDE_DIR} )
  list(APPEND LINK_LIBS ${JPEG_LIBRARY} )
endif()

find_package(TIFF QUIET)
if(TIFF_FOUND)
  set(HAVE_TIFF 1)
  list(APPEND INTERNAL_INC ${TIFF_INCLUDE_DIR} )
  list(APPEND LINK_LIBS ${TIFF_LIBRARY} )
endif()


#######################################################
## Add Libraries / Include Directories / Link directories

include_directories( ${INTERNAL_INC} )
include_directories( ${USER_INC} )
link_libraries(${LINK_LIBS})
link_directories(${LIB_DIRS})

add_library(${LIBRARY_NAME} ${SOURCES} ${HEADERS})

## Set Special Compiler flags
if(MSVC)
  set(CMAKE_CXX_FLAGS "/EHs")
elseif(CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS "-Wall -Wno-error=deprecated-declarations")
endif()

#######################################################
## Create configure file for inclusion in library

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/config.h.in"
  "${CMAKE_CURRENT_BINARY_DIR}/include/${LIBRARY_NAME}/config.h"
)

#######################################################
## Install headers / targets

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/config.h
  DESTINATION ${CMAKE_INSTALL_PREFIX}/include/${LIBRARY_NAME}
)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/${LIBRARY_NAME}
  DESTINATION ${CMAKE_INSTALL_PREFIX}/include/${LIBRARY_NAME}
)

install(TARGETS ${LIBRARY_NAME}
  RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
  LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
  ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
)

#######################################################
## Export config file for easy inclusion into other projects: find_package( Pangolin )
get_target_property( LIB_LOC ${LIBRARY_NAME} LOCATION )
configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/PangolinConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/PangolinConfig.cmake @ONLY IMMEDIATE )
export( PACKAGE Pangolin )
