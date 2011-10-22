#######################################################
## Library sources

SET(
  SOURCES
  pangolin.h
  platform.h
  simple_math.h
  display.h display_internal.h display.cpp
  vars.h vars_internal.h vars.cpp
  widgets.h widgets.cpp
  plotter.h plotter.cpp
  gl.h glcuda.h cg.h
  video.h video.cpp
  firewire.h firewire.cpp
  v4l.h v4l.cpp
  ffmpeg.h ffmpeg.cpp
)

#######################################################
## Find required dependencies

FIND_PACKAGE(OpenGL REQUIRED QUIET)
FIND_PACKAGE(Boost REQUIRED QUIET)

#######################################################
## Find optional dependencies

FIND_PACKAGE(CUDA QUIET)
IF(CUDA_FOUND)
  SET(HAVE_CUDA 1)
  CUDA_INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
  INCLUDE_DIRECTORIES( ${CUDA_SDK_ROOT_DIR}/common/inc )
  LINK_DIRECTORIES( ${CUDA_SDK_ROOT_DIR}/lib )
  LINK_DIRECTORIES( ${CUDA_SDK_ROOT_DIR}/common/lib )
  SET(CUDA_ATTACH_VS_BUILD_RULE_TO_CUDA_FILE OFF)
  SET(CUDA_NVCC_FLAGS "-O3;-arch=sm_20;--verbose" )
  ADD_DEFINITIONS(-DCUDA_BUILD)
  MESSAGE(STATUS "CUDA Found and Enabled")
ENDIF()

IF(CUDA_FOUND)
  CUDA_ADD_LIBRARY(${LIBRARY_NAME} SHARED ${SOURCES})
ELSE()
  ADD_LIBRARY(${LIBRARY_NAME} SHARED ${SOURCES})
ENDIF()

FIND_PACKAGE(FREEGLUT QUIET)
IF(FREEGLUT_FOUND)
  SET(HAVE_GLUT 1)
  INCLUDE_DIRECTORIES( ${LIBRARY_NAME} ${FREEGLUT_INCLUDE_DIR} )
  TARGET_LINK_LIBRARIES( ${LIBRARY_NAME} ${FREEGLUT_LIBRARY} )
  MESSAGE(STATUS "Freeglut Found and Enabled")
ENDIF()

FIND_PACKAGE(DC1394 QUIET)
IF(DC1394_FOUND)
  SET(HAVE_DC1394 1)
  INCLUDE_DIRECTORIES( ${LIBRARY_NAME} ${DC1394_INCLUDE_DIR} )
  TARGET_LINK_LIBRARIES( ${LIBRARY_NAME} ${DC1394_LIBRARY} )
  MESSAGE(STATUS "libdc1394 Found and Enabled")
ENDIF()

FIND_PACKAGE(FFMPEG QUIET)
IF(FFMPEG_FOUND)
  SET(HAVE_FFMPEG 1)
  INCLUDE_DIRECTORIES( ${LIBRARY_NAME} ${FFMPEG_INCLUDE_DIRS} )
  TARGET_LINK_LIBRARIES( ${LIBRARY_NAME} ${FFMPEG_LIBRARIES} )
  MESSAGE(STATUS "ffmpeg Found and Enabled")
ENDIF()

#FIND_PACKAGE(TOON QUIET)
#IF(TOON_FOUND)
  SET(HAVE_TOON 1)
#ENDIF()

## Apply required dependencies settings
INCLUDE_DIRECTORIES( ${LIBRARY_NAME} ${Boost_INCLUDE_DIRS} )
INCLUDE_DIRECTORIES( ${LIBRARY_NAME} ${OPENGL_INCLUDE_DIR} )
TARGET_LINK_LIBRARIES( ${LIBRARY_NAME} ${OPENGL_LIBRARIES} )

## Set Special Compiler flags
IF(MSVC)
  SET(CMAKE_CXX_FLAGS "/EHs")
ELSE()
  SET(CMAKE_CXX_FLAGS "-Wall")
ENDIF()

#######################################################
## Create configure file for inclusion in library

CONFIGURE_FILE(
  "${CMAKE_CURRENT_SOURCE_DIR}/config.h.in"
  "${CMAKE_CURRENT_BINARY_DIR}/config.h"
)
INCLUDE_DIRECTORIES("${PROJECT_BINARY_DIR}")

#######################################################
## Define headers files associated with the install target 
SET(INSTALL_HEADERS ${CMAKE_CURRENT_BINARY_DIR}/config.h 
	cg.h 
	display.h 
	display_internal.h 
	ffmpeg.h 
	firewire.h 
	gl.h 
	glcuda.h 
	pangolin.h 
	platform.h 
	plotter.h 
	simple_math.h 
	v4l.h 
	vars.h 
	vars_internal.h 
	video.h 
	widgets.h
)

# install headers
INSTALL(FILES ${INSTALL_HEADERS} 
  DESTINATION ${CMAKE_INSTALL_PREFIX}/include/${LIBRARY_NAME}
)

# install library
INSTALL(TARGETS ${LIBRARY_NAME}
  RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
  LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
  ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
)
