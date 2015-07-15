###############################################################################
# Find Toshiba TeliCam
#
# This sets the following variables:
# TeliCam_FOUND - True if TeliCam was found.
# TeliCam_INCLUDE_DIRS - Directories containing the TeliCam include files.
# TeliCam_LIBRARIES - Libraries needed to use TeliCam.

find_path(
    TeliCam_INCLUDE_DIR TeliCamApi.h
    PATHS
        "$ENV{PROGRAMFILES}/Toshiba Teli/TeliCamSDK/TeliCamApi/Include"
        "$ENV{PROGRAMW6432}/Toshiba Teli/TeliCamSDK/TeliCamApi/Include"
        "${CMAKE_SOURCE_DIR}/../TeliCamSDK/TeliCamApi/Include"
        /usr/include
        /user/include
    PATH_SUFFIXES TeliCam
)

find_library(
    TeliCamApi_LIBRARY
    NAMES TeliCamApi
    PATHS
        "$ENV{PROGRAMFILES}/Toshiba Teli/TeliCamSDK/TeliCamApi/lib"
        "$ENV{PROGRAMW6432}/Toshiba Teli/TeliCamSDK/TeliCamApi/lib"
        "${CMAKE_SOURCE_DIR}/../TeliCamSDK/TeliCamApi/lib"
        /usr/lib
        /user/lib
    PATH_SUFFIXES x64 x86
)

find_library(
    TeliCamUtl_LIBRARY
    NAMES TeliCamUtl
    PATHS
        "$ENV{PROGRAMFILES}/Toshiba Teli/TeliCamSDK/TeliCamApi/lib"
        "$ENV{PROGRAMW6432}/Toshiba Teli/TeliCamSDK/TeliCamApi/lib"
        "${CMAKE_SOURCE_DIR}/../TeliCamSDK/TeliCamApi/lib"
        /usr/lib
        /user/lib
    PATH_SUFFIXES x64 x86
)

set(TeliCam_INCLUDE_DIRS ${TeliCam_INCLUDE_DIR})
set(TeliCam_LIBRARY "${TeliCamApi_LIBRARY}" "${TeliCamUtl_LIBRARY}")
set(TeliCam_LIBRARIES ${TeliCam_LIBRARY})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args( TeliCam
  FOUND_VAR TeliCam_FOUND
  REQUIRED_VARS TeliCamApi_LIBRARY TeliCamUtl_LIBRARY TeliCam_INCLUDE_DIR
)
