#######################################################
## Library sources

SET(
  SOURCES
  pangolin.h
  platform.h
  simple_math.h
  display.h display_internal.h display.cpp
  plotter.h plotter.cpp
  gl.h glcuda.h cg.h
  threadedfilebuf.h threadedfilebuf.cpp
  timer.h
)

OPTION(BUILD_PANGOLIN_VARS "Build support for Pangolin Vars" ON)
IF(BUILD_PANGOLIN_VARS)
  LIST(APPEND SOURCES
    vars.h vars_internal.h vars.cpp
    input_record_repeat.h input_record_repeat.cpp
    widgets.h widgets.cpp
  )
ENDIF()

OPTION(BUILD_PANGOLIN_VIDEO "Build support for Pangolin Video Utilities" ON)
IF(BUILD_PANGOLIN_VIDEO)
  LIST(APPEND SOURCES
    video.h video.cpp
    video_recorder.h video_recorder.cpp
    video_record_repeat.h video_record_repeat.cpp
    video/pvn_video.h video/pvn_video.cpp
  )
ENDIF()

#######################################################
## Find required dependencies

FIND_PACKAGE(OpenGL REQUIRED QUIET)
FIND_PACKAGE(GLEW REQUIRED)
FIND_PACKAGE(Boost COMPONENTS thread REQUIRED QUIET)

## Apply required dependencies settings
LIST(APPEND LINK_LIBS "${OPENGL_LIBRARIES};${GLEW_LIBRARY};${Boost_LIBRARIES}" )
LIST(APPEND LIB_DIRS  "${OPENGL_LIBRARY_DIR};${GLEW_LIBRARY_DIR};${Boost_LIBRARY_DIRS};${PROJECT_BINARY_DIR}/pangolin" )
LIST(APPEND USER_INC  "${OPENGL_INCLUDE_DIR};${GLEW_INCLUDE_DIR};${Boost_INCLUDE_DIRS};${CMAKE_CURRENT_SOURCE_DIR}/..;${PROJECT_BINARY_DIR}" )

#######################################################
## Find optional dependencies

FIND_PACKAGE(CVARS QUIET)
IF(CVARS_FOUND)
  SET(HAVE_CVARS 1)
  LIST(APPEND USER_INC  ${CVARS_INCLUDE_DIR} )
  LIST(APPEND LINK_LIBS ${CVARS_LIBRARIES} )
  LIST(APPEND LIB_DIRS ${CVARS_DIR} )
ENDIF()

FIND_PACKAGE(CUDA QUIET)
IF(CUDA_FOUND)
  SET(HAVE_CUDA 1)
  CUDA_INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
  SET(CUDA_ATTACH_VS_BUILD_RULE_TO_CUDA_FILE OFF)
  SET(CUDA_NVCC_FLAGS "-O3" )
  ADD_DEFINITIONS(-DCUDA_BUILD)
  MESSAGE(STATUS "CUDA Found and Enabled")
ENDIF()

FIND_PACKAGE(FREEGLUT)
FIND_PACKAGE(GLUT)

# Prefer FREEGLUT
IF(FREEGLUT_FOUND)
  SET(HAVE_FREEGLUT 1)
  LIST(APPEND USER_INC  ${FREEGLUT_INCLUDE_DIR} )
  LIST(APPEND LINK_LIBS ${FREEGLUT_LIBRARY} )
ELSEIF(GLUT_FOUND)
  LIST(APPEND USER_INC  ${GLUT_INCLUDE_DIR} )
  LIST(APPEND LINK_LIBS ${GLUT_LIBRARY} )
  IF(_OSX_)
    SET(HAVE_APPLE_OPENGL_FRAMEWORK 1)
    MESSAGE(STATUS "Using Apple Framework GLUT. Install Freeglut for scroll support.")
  ENDIF()
ENDIF()

IF(FREEGLUT_FOUND OR GLUT_FOUND)
    SET(HAVE_GLUT 1)
    MESSAGE(STATUS "Glut Found and Enabled")
ENDIF()

FIND_PACKAGE(Eigen3)
IF(EIGEN3_FOUND)
  SET(HAVE_EIGEN 1)
  LIST(APPEND USER_INC  ${EIGEN3_INCLUDE_DIR} )
  MESSAGE(STATUS "Eigen Found and Enabled")
ENDIF()

FIND_PACKAGE(TooN QUIET)
IF(TooN_FOUND)
  SET(HAVE_TOON 1)
  LIST(APPEND USER_INC  ${TooN_INCLUDE_DIR} )
  MESSAGE(STATUS "TooN Found and Enabled")
ENDIF()

FIND_PACKAGE(DC1394 QUIET)
IF(BUILD_PANGOLIN_VIDEO AND DC1394_FOUND)
  SET(HAVE_DC1394 1)
  LIST(APPEND INTERNAL_INC  ${DC1394_INCLUDE_DIR} )
  LIST(APPEND LINK_LIBS  ${DC1394_LIBRARY} )
  LIST(APPEND SOURCES video/firewire.h video/firewire.cpp)
  MESSAGE(STATUS "libdc1394 Found and Enabled")
ENDIF()

IF(BUILD_PANGOLIN_VIDEO AND _LINUX_)
  SET(HAVE_V4L 1)
  LIST(APPEND SOURCES video/v4l.h video/v4l.cpp)
ENDIF()

FIND_PACKAGE(FFMPEG QUIET)
IF(BUILD_PANGOLIN_VIDEO AND FFMPEG_FOUND)
  SET(HAVE_FFMPEG 1)
  LIST(APPEND INTERNAL_INC  ${FFMPEG_INCLUDE_DIRS} )
  LIST(APPEND LINK_LIBS ${FFMPEG_LIBRARIES} )
  LIST(APPEND SOURCES video/ffmpeg.h video/ffmpeg.cpp)
  MESSAGE(STATUS "ffmpeg Found and Enabled")
ENDIF()

FIND_PACKAGE(OpenNI QUIET)
IF(BUILD_PANGOLIN_VIDEO AND OPENNI_FOUND)
  SET(HAVE_OPENNI 1)
  if(CMAKE_COMPILER_IS_GNUCXX)
     # Ignore warnings generated from third party includes
     set_source_files_properties( video/openni.cpp video.cpp PROPERTIES COMPILE_FLAGS " -w " )
  endif(CMAKE_COMPILER_IS_GNUCXX)
  LIST(APPEND INTERNAL_INC  ${OPENNI_INCLUDE_DIRS} )
  LIST(APPEND LINK_LIBS ${OPENNI_LIBRARIES} )
  LIST(APPEND SOURCES video/openni.h video/openni.cpp)
  MESSAGE(STATUS "OpenNI Found and Enabled")
ENDIF()

#######################################################
## Add Librarys / Include Directories / Link directories

INCLUDE_DIRECTORIES( ${INTERNAL_INC} )
INCLUDE_DIRECTORIES( ${USER_INC} )
LINK_LIBRARIES(${LINK_LIBS})
LINK_DIRECTORIES(${LIB_DIRS})

IF(CUDA_FOUND)
  CUDA_ADD_LIBRARY(${LIBRARY_NAME} ${SOURCES})
ELSE()
  ADD_LIBRARY(${LIBRARY_NAME} ${SOURCES})
ENDIF()

## Set Special Compiler flags
IF(MSVC)
  SET(CMAKE_CXX_FLAGS "/EHs")
ELSEIF(CMAKE_COMPILER_IS_GNUCXX)
#  SET(CMAKE_CXX_FLAGS "-Wall -Wno-error=deprecated-declarations -Werror")
  SET(CMAKE_CXX_FLAGS "-Wall -Wno-error=deprecated-declarations")
ENDIF()

#######################################################
## Create configure file for inclusion in library

CONFIGURE_FILE(
  "${CMAKE_CURRENT_SOURCE_DIR}/config.h.in"
  "${CMAKE_CURRENT_BINARY_DIR}/config.h"
)

#######################################################
## Define headers files associated with the install target 
SET(INSTALL_HEADERS ${CMAKE_CURRENT_BINARY_DIR}/config.h 
	cg.h 
	display.h 
	display_internal.h 
	gl.h 
	glcuda.h 
	pangolin.h 
	platform.h 
	plotter.h 
	simple_math.h 
	vars.h 
	vars_internal.h 
	video.h 
        video/firewire.h
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

# Export config file for easy inclusion into other projects: FIND_PACKAGE( Pangolin )
CONFIGURE_FILE( ${CMAKE_CURRENT_SOURCE_DIR}/PangolinConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/PangolinConfig.cmake @ONLY IMMEDIATE )
EXPORT( PACKAGE Pangolin )
