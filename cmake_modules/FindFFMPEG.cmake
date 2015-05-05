# Try to find the ffmpeg libraries and headers for avcodec avformat swscale
#
# FFMPEG_INCLUDE_DIRS
# FFMPEG_LIBRARIES
# FFMPEG_FOUND

# Find header files
FIND_PATH(
  AVCODEC_INCLUDE_DIR libavcodec/avcodec.h
  /usr/include /usr/local/include /opt/local/include
)
FIND_PATH(
  AVFORMAT_INCLUDE_DIR libavformat/avformat.h
  /usr/include /usr/local/include /opt/local/include
)
FIND_PATH(
  AVUTIL_INCLUDE_DIR libavutil/avutil.h
  /usr/include /usr/local/include /opt/local/include
)
FIND_PATH(
  SWSCALE_INCLUDE_DIR libswscale/swscale.h
  /usr/include /usr/local/include /opt/local/include
)

# Find Library files
FIND_LIBRARY(
  AVCODEC_LIBRARY
  NAMES avcodec
  PATH /usr/lib /usr/local/lib /opt/local/lib
)
FIND_LIBRARY(
  AVFORMAT_LIBRARY
  NAMES avformat
  PATH /usr/lib /usr/local/lib /opt/local/lib
)
FIND_LIBRARY(
  AVUTIL_LIBRARY
  NAMES avutil
  PATH /usr/lib /usr/local/lib /opt/local/lib
)
FIND_LIBRARY(
  SWSCALE_LIBRARY
  NAMES swscale
  PATH /usr/lib /usr/local/lib /opt/local/lib
)

IF( EXISTS "${AVUTIL_INCLUDE_DIR}/libavutil/pixdesc.h" )
  SET( AVUTIL_HAVE_PIXDESC TRUE)
endif()

IF(AVCODEC_INCLUDE_DIR AND AVFORMAT_INCLUDE_DIR AND AVUTIL_INCLUDE_DIR AND SWSCALE_INCLUDE_DIR AND AVCODEC_LIBRARY AND AVFORMAT_LIBRARY AND AVUTIL_LIBRARY AND SWSCALE_LIBRARY AND AVUTIL_HAVE_PIXDESC)
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
