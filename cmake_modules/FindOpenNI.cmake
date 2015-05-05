# -*- mode: cmake; -*-
###############################################################################
# Find OpenNI
#
# This sets the following variables:
# OPENNI_FOUND - True if OPENNI was found.
# OPENNI_INCLUDE_DIRS - Directories containing the OPENNI include files.
# OPENNI_LIBRARIES - Libraries needed to use OPENNI.
# OPENNI_DEFINITIONS - Compiler flags for OPENNI.
#
# File forked from augmented_dev, project of alantrrs
# (https://github.com/alantrrs/augmented_dev).

find_package(PkgConfig)
if(${CMAKE_VERSION} VERSION_LESS 2.8.2)
  pkg_check_modules(PC_OPENNI openni-dev)
else()
  pkg_check_modules(PC_OPENNI QUIET openni-dev)
endif()

set(OPENNI_DEFINITIONS ${PC_OPENNI_CFLAGS_OTHER})

#using the 64bit version of OpenNi if generating for 64bit
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(PROGRAMFILES_ "$ENV{PROGRAMW6432}")
    set(OPENNI_SUFFIX "64")
else(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(PROGRAMFILES_ "$ENV{PROGRAMFILES}")
    set(OPENNI_SUFFIX "")
endif(CMAKE_SIZEOF_VOID_P EQUAL 8)

#add a hint so that it can find it without the pkg-config
find_path(OPENNI_INCLUDE_DIR XnStatus.h
    HINTS ${PC_OPENNI_INCLUDEDIR} ${PC_OPENNI_INCLUDE_DIRS} /usr/include/ni /usr/include/openni
    "${PROGRAMFILES_}/OpenNI/Include"
    PATH_SUFFIXES openni)
#add a hint so that it can find it without the pkg-config
find_library(OPENNI_LIBRARY
    NAMES OpenNI64 OpenNI
    HINTS ${PC_OPENNI_LIBDIR} ${PC_OPENNI_LIBRARY_DIRS} /usr/lib "${PROGRAMFILES_}/OpenNI/Lib${OPENNI_SUFFIX}")

set(OPENNI_INCLUDE_DIRS ${OPENNI_INCLUDE_DIR})
set(OPENNI_LIBRARIES ${OPENNI_LIBRARY})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(OpenNI DEFAULT_MSG
    OPENNI_LIBRARY OPENNI_INCLUDE_DIR)

mark_as_advanced(OPENNI_LIBRARY OPENNI_INCLUDE_DIR)
