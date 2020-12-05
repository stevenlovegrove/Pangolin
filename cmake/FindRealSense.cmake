# -*- mode: cmake; -*-
###############################################################################
# Find realsense  https://github.com/IntelRealSense/librealsense
#
# This sets the following variables:
# REALSENSE_FOUND - True if RealSense was found.
# REALSENSE_INCLUDE_DIRS - Directories containing the RealSense include files.
# REALSENSE_LIBRARIES - Libraries needed to use RealSense.
# REALSENSE_DEFINITIONS - Compiler flags for RealSense.
#
# File forked from augmented_dev, project of alantrrs
# (https://github.com/alantrrs/augmented_dev).

find_package(PkgConfig)
if(${CMAKE_VERSION} VERSION_LESS 2.8.2)
endif()

#add a hint so that it can find it without the pkg-config
find_path(REALSENSE_INCLUDE_DIR librealsense/rs.h
    HINTS /usr/include/  /usr/local/include)
#add a hint so that it can find it without the pkg-config
find_library(REALSENSE_LIBRARY
    NAMES realsense
    HINTS /usr/lib /usr/local/lib )

  set(REALSENSE_INCLUDE_DIRS ${REALSENSE_INCLUDE_DIR})
  set(REALSENSE_LIBRARIES ${REALSENSE_LIBRARY})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(RealSense DEFAULT_MSG
  REALSENSE_LIBRARY REALSENSE_INCLUDE_DIR)

mark_as_advanced(REALSENSE_LIBRARY REALSENSE_INCLUDE_DIR)
