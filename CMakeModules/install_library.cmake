################################################################################
# install_library.cmake - Functions for easy pkg-config'd library installation
# 
# This will create and install a ${PACKAGE}.pc pkg-config file.
# Will also create a Find${PACKAGE}.cmake, which will in turn call.

#
# install_package - Takes a package name and the following optional arguments:
#  LIBRARY scenegraph
#  VERSION version
#  HEADERS ${SceneGraph_HDRS}
#  INCLUDE_DIRS ${REQUIRED_INCLUDE_DIRS}
#  LIB_DEPENDS ${REQUIRED_LIBRARIES}
#  LINK_DIRS ${REQUIRED_LIBRARIES}
#
################################################################################
include(CMakeParseArguments)

get_filename_component(modules_dir ${CMAKE_CURRENT_LIST_FILE} PATH)

function(install_library)
  set(PACKAGE_OPTIONS "")
  set(PACKAGE_SINGLE_ARGS "")
  set( PACKAGE_MULTI_ARGS LIBRARY VERSION DESCRIPTION HEADERS
      HEADER_DESTINATION INCLUDE_DIRS LIB_DEPENDS LINK_DIRS )
  cmake_parse_arguments( PACKAGE
    "${PACKAGE_OPTIONS}"
    "${PACKAGE_SINGLE_ARGS}"
    "${PACKAGE_MULTI_ARGS}"
    "${ARGN}"
    )

  string(TOUPPER ${PACKAGE_LIBRARY} PACKAGE)

  # clean things up 
  if( PACKAGE_LINK_DIRS )
    list( REMOVE_DUPLICATES PACKAGE_LINK_DIRS )
  endif()
  if(PACKAGE_LIB_DEPENDS)
    list( REMOVE_DUPLICATES PACKAGE_LIB_DEPENDS )
  endif()
  if( PACKAGE_INCLUDE_DIRS)
    list( REMOVE_DUPLICATES PACKAGE_INCLUDE_DIRS )
  endif()

  # add "installed" library to list of required libraries to link against
  cmake_policy( SET CMP0026 OLD )
  get_target_property( _target_library ${PACKAGE_LIBRARY} LOCATION )
  get_filename_component( _lib ${_target_library} NAME )
  list( APPEND PACKAGE_LIB_DEPENDS ${CMAKE_INSTALL_PREFIX}/lib/${_lib} )

  message( STATUS "depends: ${PACKAGE_LIB_DEPENDS}" )



  # construct Cflags arguments for pkg-config file
  string( CONCAT PACKAGE_CFLAGS ${PACKAGE_CFLAGS} ${CMAKE_C_FLAGS} )
  foreach(var IN LISTS PACKAGE_INCLUDE_DIRS )
    string( CONCAT PACKAGE_CFLAGS ${PACKAGE_CFLAGS} " -I${var}" )
  endforeach()

  # now construct Libs.private arguments 
  foreach(var IN LISTS PACKAGE_LINK_DIRS )
    string( CONCAT PACKAGE_PRIVATE_LIBS ${PACKAGE_PRIVATE_LIBS} " -L${var}" )
  endforeach()
  foreach(var IN LISTS PACKAGE_LIB_DEPENDS )
    if( EXISTS ${var} OR  ${var} MATCHES "-framework*" )
      string( CONCAT PACKAGE_PRIVATE_LIBS ${PACKAGE_PRIVATE_LIBS} " ${var}" )
    else() # assume it's just a -l call??
      string( CONCAT PACKAGE_PRIVATE_LIBS ${PACKAGE_PRIVATE_LIBS} " -l${var}" )
    endif()
  endforeach()

  # build pkg-config file
  configure_file( ${modules_dir}/PkgConfig.pc.in ${PACKAGE_LIBRARY}.pc @ONLY )

  # install pkg-config file for external projects to discover this library
  install( FILES ${CMAKE_CURRENT_BINARY_DIR}/${PACKAGE_LIBRARY}.pc 
      DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/pkgconfig/ )

  # install header files
  install( FILES ${PACKAGE_HEADERS} DESTINATION ${PACKAGE_HEADER_DESTINATION} )

  # install library itself
  install( FILES ${_target_library} DESTINATION ${CMAKE_INSTALL_PREFIX}/lib )

  # write and install a cmake "find package" for cmake projects to use.
  configure_file( ${modules_dir}/FindPackage.cmake.in Find${PACKAGE}.cmake @ONLY )
  install( FILES ${CMAKE_CURRENT_BINARY_DIR}/Find${PACKAGE}.cmake 
      DESTINATION ${CMAKE_INSTALL_PREFIX}/share/cmake/Modules )

endfunction()


