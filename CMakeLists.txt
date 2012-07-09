PROJECT("Pangolin")
SET(PANGOLIN_VERSION_MAJOR 0)
SET(PANGOLIN_VERSION_MINOR 1)

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/CMakeModules/")

OPTION(BUILD_EXAMPLES "Build Examples" ON)
OPTION(BUILD_SHARED_LIBS OFF)

# Overide with cmake -DCMAKE_BUILD_TYPE=Debug {dir}
IF( NOT CMAKE_BUILD_TYPE )
   MESSAGE("Build type not set (defaults to release)")
   MESSAGE("-DCMAKE_BUILD_TYPE=Debug for debug")
   SET( CMAKE_BUILD_TYPE Release )
ENDIF()

# Platform configuration vars
INCLUDE(SetPlatformVars)

STRING(TOLOWER ${PROJECT_NAME} LIBRARY_NAME)

# make an uninstall target 
CONFIGURE_FILE(
 	"${CMAKE_SOURCE_DIR}/cmake_uninstall.cmake.in"
  	"${CMAKE_SOURCE_DIR}/cmake_uninstall.cmake"
  	IMMEDIATE @ONLY
)

ADD_CUSTOM_TARGET(uninstall
  "${CMAKE_COMMAND}" -P "${CMAKE_SOURCE_DIR}/cmake_uninstall.cmake")

ADD_SUBDIRECTORY(${LIBRARY_NAME})

IF(BUILD_EXAMPLES)
  ADD_SUBDIRECTORY(examples)
ENDIF()
