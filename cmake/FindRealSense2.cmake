# -*- mode: cmake; -*-
###############################################################################
# Find realsense2  https://github.com/IntelRealSense/librealsense
#
# This sets the following variables:
# REALSENSE2_FOUND - True if REALSENSE2 was found.
# REALSENSE2_INCLUDE_DIRS - Directories containing the REALSENSE2 include files.
# REALSENSE2_LIBRARIES - Libraries needed to use REALSENSE2.
# REALSENSE2_DEFINITIONS - Compiler flags for REALSENSE2.
#
# File forked from augmented_dev, project of alantrrs
# (https://github.com/alantrrs/augmented_dev).

find_package(PkgConfig)
if(${CMAKE_VERSION} VERSION_LESS 2.8.2)
endif()

#add a hint so that it can find it without the pkg-config
find_path(REALSENSE2_INCLUDE_DIR librealsense2/rs.h
    HINTS /usr/include/  /usr/local/include)
#add a hint so that it can find it without the pkg-config
find_library(REALSENSE2_LIBRARY
    NAMES realsense2
    HINTS /usr/lib /usr/local/lib )

  set(REALSENSE2_INCLUDE_DIRS ${REALSENSE2_INCLUDE_DIR})
  set(REALSENSE2_LIBRARIES ${REALSENSE2_LIBRARY})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(RealSense2 DEFAULT_MSG
  REALSENSE2_LIBRARY REALSENSE2_INCLUDE_DIR)

mark_as_advanced(REALSENSE2_LIBRARY REALSENSE2_INCLUDE_DIR)
