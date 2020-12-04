###############################################################################
# Find OpenNI
#
#     find_package(OpenNI)
#
# Variables defined by this module:
#
#  OPENNI_FOUND                True if OpenNI was found
#  OPENNI_INCLUDE_DIRS         The location(s) of OpenNI headers
#  OPENNI_LIBRARIES            Libraries needed to use OpenNI
#  OPENNI_DEFINITIONS          Compiler flags for OpenNI

find_package(PkgConfig QUIET)
pkg_check_modules(PC_OPENNI QUIET libopenni)

set(OPENNI_DEFINITIONS ${PC_OPENNI_CFLAGS_OTHER})

set(OPENNI_SUFFIX)
if(WIN32 AND CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(OPENNI_SUFFIX 64)
endif()

# Add a hint so that it can find it without the pkg-config
find_path(OPENNI_INCLUDE_DIR XnStatus.h
    HINTS ${PC_OPENNI_INCLUDEDIR}
    ${PC_OPENNI_INCLUDE_DIRS}
    /usr/include/openni
    /usr/include/ni
    /opt/local/include/ni
    "${OPENNI_ROOT}"
    "$ENV{OPENNI_ROOT}"
    PATHS "$ENV{OPEN_NI_INSTALL_PATH${OPENNI_SUFFIX}}/Include"
    PATH_SUFFIXES openni include Include)

# Add a hint so that it can find it without the pkg-config
find_library(OPENNI_LIBRARY
    NAMES OpenNI${OPENNI_SUFFIX}
    HINTS ${PC_OPENNI_LIBDIR}
    ${PC_OPENNI_LIBRARY_DIRS}
    /usr/lib
    "${OPENNI_ROOT}"
    "$ENV{OPENNI_ROOT}"
    PATHS "$ENV{OPEN_NI_LIB${OPENNI_SUFFIX}}"
    PATH_SUFFIXES lib Lib Lib64)

if(OPENNI_INCLUDE_DIR AND OPENNI_LIBRARY)
    set(OPENNI_INCLUDE_DIRS ${OPENNI_INCLUDE_DIR})
    mark_as_advanced(OPENNI_INCLUDE_DIRS)

    set(OPENNI_LIBRARIES ${OPENNI_LIBRARY})
    mark_as_advanced(OPENNI_LIBRARIES)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(OpenNI
    FOUND_VAR OPENNI_FOUND
    REQUIRED_VARS OPENNI_LIBRARIES OPENNI_INCLUDE_DIRS
    )
