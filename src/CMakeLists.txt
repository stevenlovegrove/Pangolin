#######################################################
## Library sources

set( INCDIR "../include/pangolin")

## Header only includes / core headers
set(
  HEADERS
  ${INCDIR}/pangolin.h ${INCDIR}/platform.h
  ${INCDIR}/simple_math.h ${INCDIR}/timer.h
  ${INCDIR}/gl.h ${INCDIR}/glinclude.h ${INCDIR}/gl_es_compat.h
  ${INCDIR}/gldraw.h ${INCDIR}/glvbo.h
  ${INCDIR}/glcuda.h ${INCDIR}/cg.h ${INCDIR}/glsl.h
  ${INCDIR}/image.h
  ${INCDIR}/file_utils.h
)

## Store list of source files
set( SOURCES file_utils.cpp image_load.cpp video_common.cpp )

#######################################################
## User build options

option(BUILD_PANGOLIN_GUI "Build support for Pangolin GUI" ON)
if(BUILD_PANGOLIN_GUI)
  list(APPEND HEADERS
    ${INCDIR}/display.h ${INCDIR}/display_internal.h 
    ${INCDIR}/view.h ${INCDIR}/viewport.h
    ${INCDIR}/handler.h ${INCDIR}/handler_enums.h
    ${INCDIR}/handler_glbuffer.h
    ${INCDIR}/opengl_render_state.h
    ${INCDIR}/datalog.h
    ${INCDIR}/glfont.h ${INCDIR}/glpangoglu.h
  )
  list(APPEND SOURCES
    display.cpp view.cpp viewport.cpp
    handler.cpp handler_glbuffer.cpp
    opengl_render_state.cpp
    datalog.cpp
    glfont.cpp glpangoglu.cpp
  )
  if(NOT HAVE_GLES OR HAVE_GLES_2)
    list(APPEND HEADERS ${INCDIR}/plotter.h)
    list(APPEND SOURCES plotter.cpp)
  endif()
endif()

option(BUILD_PANGOLIN_VARS "Build support for Pangolin Vars" ON)
if(BUILD_PANGOLIN_VARS)
  list(APPEND HEADERS ${INCDIR}/var/var.h ${INCDIR}/var/varextra.h
    ${INCDIR}/var/varstate.h ${INCDIR}/var/varvalue.h
    ${INCDIR}/var/varvaluegeneric.h ${INCDIR}/var/varvaluet.h
    ${INCDIR}/var/varwrapper.h ${INCDIR}/input_record_repeat.h
  )
  list(APPEND SOURCES var/vars.cpp input_record_repeat.cpp )

  if(BUILD_PANGOLIN_GUI)
      list(APPEND HEADERS ${INCDIR}/widgets.h )
      list(APPEND SOURCES widgets.cpp )
  endif()
endif()

option(BUILD_PANGOLIN_VIDEO "Build support for Pangolin Video Utilities" ON)
if(BUILD_PANGOLIN_VIDEO)
  list(APPEND HEADERS
    ${INCDIR}/threadedfilebuf.h ${INCDIR}/video.h ${INCDIR}/video_common.h
    ${INCDIR}/video_splitter.h ${INCDIR}/video_output.h
    ${INCDIR}/video_recorder.h ${INCDIR}/video_record_repeat.h
    ${INCDIR}/video/pvn_video.h
    ${INCDIR}/video/test.h ${INCDIR}/video/images.h
  )
  list(APPEND SOURCES
    threadedfilebuf.cpp video.cpp video_splitter.cpp video_output.cpp
    video_recorder.cpp video_record_repeat.cpp video/pvn_video.cpp
    video/test.cpp video/images.cpp
  )
endif()

#######################################################
## Setup required includes / link info

# Project headers trump everything (including any potentially installed Pangolin)
list(APPEND LIB_INC_DIR  "${PROJECT_SOURCE_DIR}/include;${CMAKE_CURRENT_BINARY_DIR}/include" )

if(NOT CPP11_NO_BOOST)
    # Boost is required when c++11 is disabled
    find_package(Boost COMPONENTS thread filesystem system REQUIRED QUIET)
    list(APPEND USER_INC  "${Boost_INCLUDE_DIRS}" )
    list(APPEND LINK_LIBS "${Boost_LIBRARIES}" )

    # Generic Image Library (GIL) should come with boost.
    # If you have a version of boost without it, maybe figure out how to test here.
    set(HAVE_BOOST_GIL 1)
else()
    find_package(Boost QUIET)
    if(Boost_FOUND)
        list(APPEND USER_INC "${Boost_INCLUDE_DIRS}" )
        set(HAVE_BOOST_GIL 1)
    endif()
endif()

if(BUILD_PANGOLIN_GUI)
    if( ANDROID  )
        # Android specific display code
        list(APPEND HEADERS ${INCDIR}/display_android.h )
        list(APPEND SOURCES display_android.cpp )

        if(HAVE_GLES_2)
            list(APPEND LINK_LIBS "-lEGL;-lGLESv2" )
        else()
            list(APPEND LINK_LIBS "-lEGL;-lGLESv1_CM" )
        endif()
    elseif( IOS )
        list(APPEND LINK_LIBS "-framework OpenGLES" )
        list(APPEND HEADERS "${INCDIR}/ios/PangolinAppDelegate.h" "${INCDIR}/ios/PangolinViewController.h" )
        list(APPEND SOURCES "ios/PangolinAppDelegate.mm" "ios/PangolinViewController.mm" )
    else()
        find_package(OpenGL REQUIRED QUIET)
        list(APPEND USER_INC  "${OPENGL_INCLUDE_DIR}" )
        list(APPEND LINK_LIBS "${OPENGL_LIBRARIES}" )

        find_package(GLEW REQUIRED QUIET)
        if(GLEW_FOUND)
            list(APPEND USER_INC  "${GLEW_INCLUDE_DIR}" )
            list(APPEND LINK_LIBS "${GLEW_LIBRARY}" )
            set(HAVE_GLEW 1)
        endif()
    endif()

    if( HAVE_GLES_2 )
        # Add Pangolins backwards compat layer.
        list(APPEND HEADERS ${INCDIR}/gl2engine.h )
        list(APPEND SOURCES gl2engine.cpp)
    endif()
endif()

#######################################################
## Find optional dependencies

if(ANDROID)
    # Fix issue with thread local storage on android.
    add_definitions(-fno-data-sections)
    list(APPEND LINK_LIBS android log)
elseif(IOS)
    # Nothing specific in here yet.
else()
    find_package(CUDA QUIET)
    if(CUDA_FOUND AND NOT _CLANG_)
      set(HAVE_CUDA 1)
      cuda_include_directories(${CMAKE_CURRENT_SOURCE_DIR})
      set(CUDA_ATTACH_VS_BUILD_RULE_TO_CUDA_FILE OFF)
      set(CUDA_NVCC_FLAGS "-O3" )
      add_definitions(-DCUDA_BUILD)
      message(STATUS "CUDA Found and Enabled")
    endif()
    
    find_package(FREEGLUT QUIET)
    find_package(GLUT QUIET)
    
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
        list(APPEND HEADERS ${INCDIR}/display_glut.h )
        list(APPEND SOURCES display_glut.cpp )
        message(STATUS "Glut Found and Enabled")
    endif()
endif()

find_package(CVARS 2.3 QUIET)
if(CVARS_FOUND)
  set(HAVE_CVARS 1)
  list(APPEND USER_INC  ${CVARS_INCLUDE_DIR} )
  list(APPEND LINK_LIBS ${CVARS_LIBRARIES} )
  message(STATUS "CVARS Found and Enabled")
endif()

find_package(Oculus QUIET)
if(Oculus_FOUND)
  set(HAVE_OCULUS 1)
  list(APPEND HEADERS ${INCDIR}/hud/oculus_hud.h )
  list(APPEND SOURCES hud/oculus_hud.cpp )
  list(APPEND USER_INC ${Oculus_INCLUDE_DIRS} )
  list(APPEND LINK_LIBS ${Oculus_LIBRARIES} )
  message(STATUS "Oculus Rift Found and Enabled")
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
  list(APPEND HEADERS ${INCDIR}/video/firewire.h ${INCDIR}/video/firewire_deinterlace.h )
  list(APPEND SOURCES video/firewire.cpp video/firewire_deinterlace.cpp)
  message(STATUS "libdc1394 Found and Enabled")
endif()

if(BUILD_PANGOLIN_VIDEO AND _LINUX_)
  set(HAVE_V4L 1)
  list(APPEND HEADERS ${INCDIR}/video/v4l.h)
  list(APPEND SOURCES video/v4l.cpp)
  message(STATUS "V4L Found and Enabled")
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

find_package(OpenNI2 QUIET)
if(BUILD_PANGOLIN_VIDEO AND OPENNI2_FOUND)
  set(HAVE_OPENNI2 1)
  list(APPEND INTERNAL_INC  ${OPENNI2_INCLUDE_DIRS} )
  list(APPEND LINK_LIBS ${OPENNI2_LIBRARIES} )
  list(APPEND HEADERS ${INCDIR}/video/openni2.h )
  list(APPEND SOURCES video/openni2.cpp)
  message(STATUS "OpenNI2 Found and Enabled")
endif()

find_package(uvc QUIET)
if(BUILD_PANGOLIN_VIDEO AND uvc_FOUND)
  set(HAVE_UVC 1)
  list(APPEND INTERNAL_INC  ${uvc_INCLUDE_DIRS} )
  list(APPEND LINK_LIBS ${uvc_LIBRARIES} )
  list(APPEND HEADERS ${INCDIR}/video/uvc.h )
  list(APPEND SOURCES video/uvc.cpp)
  message(STATUS "libuvc Found and Enabled")
endif()

find_package(DepthSense QUIET)
if(BUILD_PANGOLIN_VIDEO AND DepthSense_FOUND)
  set(HAVE_DEPTHSENSE 1)
  list(APPEND INTERNAL_INC  ${DepthSense_INCLUDE_DIRS} )
  list(APPEND LINK_LIBS ${DepthSense_LIBRARIES} )
  list(APPEND HEADERS ${INCDIR}/video/depthsense.h )
  list(APPEND SOURCES video/depthsense.cpp)
  message(STATUS "DepthSense Found and Enabled")
endif()

find_package(PNG QUIET)
if(PNG_FOUND)
  set(HAVE_PNG 1)
  list(APPEND INTERNAL_INC ${PNG_INCLUDE_DIR} )
  list(APPEND LINK_LIBS ${PNG_LIBRARY} )
  message(STATUS "libpng Found and Enabled")
endif()

find_package(JPEG QUIET)
if(JPEG_FOUND)
  set(HAVE_JPEG 1)
  list(APPEND INTERNAL_INC ${JPEG_INCLUDE_DIR} )
  list(APPEND LINK_LIBS ${JPEG_LIBRARY} )
  message(STATUS "libjpeg Found and Enabled")
endif()

find_package(TIFF QUIET)
if(TIFF_FOUND)
  set(HAVE_TIFF 1)
  list(APPEND INTERNAL_INC ${TIFF_INCLUDE_DIR} )
  list(APPEND LINK_LIBS ${TIFF_LIBRARY} )
  message(STATUS "libtiff Found and Enabled")
endif()


#######################################################
## Add Libraries / Include Directories / Link directories

include_directories( ${LIB_INC_DIR} )
include_directories( ${USER_INC} )
include_directories( ${INTERNAL_INC} )

add_library(${LIBRARY_NAME} ${SOURCES} ${HEADERS})
target_link_libraries(${LIBRARY_NAME} ${LINK_LIBS})

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
## Create config.h file for inclusion in library

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/config.h.in"
  "${CMAKE_CURRENT_BINARY_DIR}/include/${LIBRARY_NAME}/config.h"
)

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

# This relative path allows installed files to be relocatable.
set( CMAKECONFIG_INSTALL_DIR lib/cmake/${PROJECT_NAME} )
file( RELATIVE_PATH REL_INCLUDE_DIR
    "${CMAKE_INSTALL_PREFIX}/${CMAKECONFIG_INSTALL_DIR}"
    "${CMAKE_INSTALL_PREFIX}/include"
)

# Export library for easy inclusion from other cmake projects. APPEND allows
# call to function even as subdirectory of larger project.
FILE(REMOVE "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake")
export( TARGETS ${LIBRARY_NAME}
        APPEND FILE "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake" )

# Version information
configure_file(${PROJECT_NAME}ConfigVersion.cmake.in
  "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake" @ONLY)

# Build tree config
set( EXPORT_LIB_INC_DIR ${LIB_INC_DIR} )
configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}Config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake @ONLY IMMEDIATE )

# Install tree config
set( EXPORT_LIB_INC_DIR "\${PROJECT_CMAKE_DIR}/${REL_INCLUDE_DIR}" )
configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}Config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/${PROJECT_NAME}Config.cmake @ONLY )

# Add package to CMake package registery for use from the build tree
option( EXPORT_${PROJECT_NAME}
  "Should the ${PROJECT_NAME} package be exported for use by other software" ON )

if( EXPORT_${PROJECT_NAME} )
  export( PACKAGE ${PROJECT_NAME} )
endif()

#######################################################
## Install headers / targets

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/include/${LIBRARY_NAME}/config.h"
  DESTINATION ${CMAKE_INSTALL_PREFIX}/include/${LIBRARY_NAME}
)
install(DIRECTORY ${INCDIR}
  DESTINATION ${CMAKE_INSTALL_PREFIX}/include
)
install(TARGETS ${LIBRARY_NAME}
  EXPORT ${PROJECT_NAME}Targets
  RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
  LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
  ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
)

#######################################################
## Install CMake config

install(
    FILES "${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/${PROJECT_NAME}Config.cmake"
          "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
    DESTINATION ${CMAKECONFIG_INSTALL_DIR}
)
install(
    EXPORT ${PROJECT_NAME}Targets DESTINATION ${CMAKECONFIG_INSTALL_DIR}
)
