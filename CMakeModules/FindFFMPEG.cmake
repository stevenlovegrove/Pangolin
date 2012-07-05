# Try to find the ffmpeg libraries and headers for avcodec avformat swscale
#
# FFMPEG_INCLUDE_DIRS
# FFMPEG_LIBRARIES
# FFMPEG_FOUND

# Find header files
FIND_PATH(
  AVCODEC_INCLUDE_DIR avcodec.h
  /usr/include/libavcodec /usr/local/include/libavcodec
  /usr/include /usr/local/include /opt/local/include/libavcodec
)
FIND_PATH(
  AVFORMAT_INCLUDE_DIR avformat.h
  /usr/include/libavformat /usr/local/include/libavformat
  /usr/include /usr/local/include /opt/local/include/libavformat
)
FIND_PATH(
  AVUTIL_INCLUDE_DIR avutil.h
  /usr/include/libavutil /usr/local/include/libavutil
  /usr/include /usr/local/include /opt/local/include/libavutil
)
FIND_PATH(
  SWSCALE_INCLUDE_DIR swscale.h
  /usr/include/libswscale /usr/local/include/libswscale
  /usr/include /usr/local/include /opt/local/include/libswscale
)

# Find Library files
FIND_LIBRARY(
  AVCODEC_LIBRARY
  NAMES avcodec
  PATH /usr/lib /usr/local/lib
)
FIND_LIBRARY(
  AVFORMAT_LIBRARY
  NAMES avformat
  PATH /usr/lib /usr/local/lib
)
FIND_LIBRARY(
  AVUTIL_LIBRARY
  NAMES avutil
  PATH /usr/lib /usr/local/lib
)
FIND_LIBRARY(
  SWSCALE_LIBRARY
  NAMES swscale
  PATH /usr/lib /usr/local/lib
)
FIND_LIBRARY(
  AVUTIL_LIBRARY
  NAMES avutil
  PATH /usr/lib /usr/local/lib
)

INCLUDE(CheckIncludeFiles)
CHECK_INCLUDE_FILES("${AVUTIL_INCLUDE_DIR}/pixdesc.h" AVUTIL_HAVE_PIXDESC)

IF(AVCODEC_INCLUDE_DIR AND AVFORMAT_INCLUDE_DIR AND AVUTIL_INCLUDE_DIR AND SWSCALE_INCLUDE_DIR AND AVCODEC_LIBRARY AND AVFORMAT_LIBRARY AND AVUTIL_LIBRARY AND SWSCALE_LIBRARY AND AVUTIL_LIBRARY AND AVUTIL_HAVE_PIXDESC)
   SET(FFMPEG_FOUND TRUE)
   SET(FFMPEG_LIBRARIES ${AVCODEC_LIBRARY} ${AVFORMAT_LIBRARY} ${AVUTIL_LIBRARY} ${SWSCALE_LIBRARY})
   SET(FFMPEG_INCLUDE_DIRS ${AVCODEC_INCLUDE_DIR} ${AVFORMAT_INCLUDE_DIR} ${AVUTIL_INCLUDE_DIR} ${SWSCALE_INCLUDE_DIR})
ENDIF()

IF (FFMPEG_FOUND)
   IF (NOT FFMPEG_FIND_QUIETLY)
      MESSAGE(STATUS "Found FFMPEG: ${FFMPEG_LIBRARIES}")
   ENDIF (NOT FFMPEG_FIND_QUIETLY)
ELSE (FFMPEG_FOUND)
   IF (FFMPEG_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "Could not find FFMPEG")
   ENDIF (FFMPEG_FIND_REQUIRED)
ENDIF (FFMPEG_FOUND)
