# - Try to find libraw
#
#  libraw_FOUND - system has libraw
#  libraw_INCLUDE_DIRS - the libraw include directories
#  libraw_LIBRARIES - link these to use libraw

FIND_PATH(
  libraw_INCLUDE_DIRS
  NAMES libraw/libraw.h
  PATHS
    ${LIBRAW_DIR}
    ${LIBRAW_DIR}/include
    /usr/include/
    /usr/local/include
    /opt/local/include
)

FIND_LIBRARY(
  libraw_LIBRARIES
  NAMES raw_r
  PATHS
    ${LIBRAW_DIR}
    ${LIBRAW_DIR}/lib
    /usr/lib
    /usr/local/lib
    /opt/local/lib
)

IF (libraw_INCLUDE_DIRS AND libraw_LIBRARIES)
   SET(libraw_FOUND TRUE)
ENDIF (libraw_INCLUDE_DIRS AND libraw_LIBRARIES)

IF (libraw_FOUND)
   IF (NOT libraw_FIND_QUIETLY)
      MESSAGE(STATUS "Found libraw: ${libraw_LIBRARIES}")
   ENDIF (NOT libraw_FIND_QUIETLY)
ELSE (libraw_FOUND)
   IF (libraw_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "Could not find libraw")
   ENDIF (libraw_FIND_REQUIRED)
ENDIF (libraw_FOUND)
