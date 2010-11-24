#######################################################
## Library sources

SET(
  SOURCES
  pangolin.h
  platform.h
  simple_math.h
  gl.h gl_internal.h gl.cpp
  vars.h
  widgets.h widgets.cpp
)

#######################################################
## Find required dependencies

FIND_PACKAGE(OpenGL REQUIRED QUIET)

TARGET_LINK_LIBRARIES( ${LIBRARY_NAME} ${OpenGL_LIBRARY} )

#######################################################
## Find optional dependencies

FIND_PACKAGE(CUDA QUIET)
IF(CUDA_FOUND)
  CUDA_INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
  SET(HAVE_CUDA 1)
  SET(CUDA_ATTACH_VS_BUILD_RULE_TO_CUDA_FILE OFF)
  INCLUDE_DIRECTORIES( ${CUDA_SDK_ROOT_DIR}/common/inc )
  LINK_DIRECTORIES( ${CUDA_SDK_ROOT_DIR}/common/lib )
  SET(CUDA_NVCC_FLAGS "-O3;-arch=sm_20;--verbose" )
  MESSAGE(STATUS "CUDA Found and Enabled")
ENDIF()

IF(CUDA_FOUND)
  CUDA_ADD_LIBRARY(${LIBRARY_NAME} ${SOURCES})
ELSE()
  ADD_LIBRARY(${LIBRARY_NAME} ${SOURCES})
ENDIF()

FIND_PACKAGE(FREEGLUT QUIET)
IF(FREEGLUT_FOUND)
  SET(HAVE_GLUT 1)
  TARGET_LINK_LIBRARIES( ${LIBRARY_NAME} ${FREEGLUT_LIBRARY} GLU )
  MESSAGE(STATUS "Freeglut Found and Enabled")
ENDIF()

#FIND_PACKAGE(ROBOTVISION QUIET)
IF(ROBOTVISION_FOUND)
  INCLUDE_DIRECTORIES(${ROBOTVISION_INCLUDE_DIR})
  TARGET_LINK_LIBRARIES( ${LIBRARY_NAME} ${ROBOTVISION_LIBRARY} )
  SET(HAVE_ROBOTVISION 1)
  MESSAGE(STATUS "RobotVision Found and Enabled")
ENDIF()

#######################################################
## Create configure file for inclusion in library

CONFIGURE_FILE(
  "${CMAKE_CURRENT_SOURCE_DIR}/config.h.in"
  "${CMAKE_CURRENT_BINARY_DIR}/config.h"
)
INCLUDE_DIRECTORIES("${PROJECT_BINARY_DIR}")

#######################################################
## Compiler options

SET(CMAKE_CXX_FLAGS "-Wall")
