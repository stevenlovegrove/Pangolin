################################################################################
# install_package.cmake - This function will install and "export" your library
#   or files in such a way that they can be found using either CMake's
#   "FindXXX.cmake" mechanism or with pkg-config.  This makes your code boradly
#   compatible with traditional unix best practices, and also easy to use from
#   other CMake projets.
# 
# This function will create and install a ${PACKAGE}.pc pkg-config file.
# Will also create a Find${PACKAGE}.cmake, which will in turn call.

#
# install_package - Takes a package name and the following optional named arguments:
#  PKG_NAME <name of the package for pkg-config>, usually the same as ${PROJECT_NAME}
#  LIB_NAME <name of a library to build, if any>
#  VERSION <version>
#  INSTALL_HEADERS <header files to install, if any>
#  DESTINATION <directory to install headers>
#  INCLUDE_DIRS <list of required include directories, if any>
#  LINK_LIBS <list of required link libraries, if any >
#  LINK_DIRS <list of required link directories, if any>
#  CFLAGS <optional list of REQUIRED c flags>
#  CXXFLAGS <optional list of REQUIRED c++ flags>
#
################################################################################
include(CMakeParseArguments)

get_filename_component(modules_dir ${CMAKE_CURRENT_LIST_FILE} PATH)

function(install_package)
  set(PACKAGE_OPTIONS "")
  set(PACKAGE_SINGLE_ARGS "")
  set( PACKAGE_MULTI_ARGS 
      PKG_NAME 
      LIB_NAME 
      VERSION
      DESCRIPTION
      INSTALL_HEADERS 
      DESTINATION
      INCLUDE_DIRS
      LINK_LIBS
      LINK_DIRS
      CFLAGS
      CXXFLAGS
     )
  cmake_parse_arguments( PACKAGE
    "${PACKAGE_OPTIONS}"
    "${PACKAGE_SINGLE_ARGS}"
    "${PACKAGE_MULTI_ARGS}"
    "${ARGN}"
    )


  # clean things up 
  if( PACKAGE_LINK_DIRS )
    list( REMOVE_DUPLICATES PACKAGE_LINK_DIRS )
  endif()
  if(PACKAGE_LINK_LIBS)
    list( REMOVE_DUPLICATES PACKAGE_LINK_LIBS )
  endif()
  if( PACKAGE_INCLUDE_DIRS)
    list( REMOVE_DUPLICATES PACKAGE_INCLUDE_DIRS )
  endif()

  # add "installed" library to list of required libraries to link against
  if( PACKAGE_LIB_NAME )
    cmake_policy( SET CMP0026 OLD )
    get_target_property( _target_library ${PACKAGE_LIB_NAME} LOCATION )
    get_filename_component( _lib ${_target_library} NAME )
    list( APPEND PACKAGE_LINK_LIBS ${CMAKE_INSTALL_PREFIX}/lib/${_lib} )
  endif()

  # construct Cflags arguments for pkg-config file
  string( CONCAT PACKAGE_CFLAGS ${PACKAGE_CFLAGS} ${CMAKE_C_FLAGS} )
  foreach(var IN LISTS PACKAGE_INCLUDE_DIRS )
    string( CONCAT PACKAGE_CFLAGS ${PACKAGE_CFLAGS} " -I${var}" )
  endforeach()

  # now construct Libs.private arguments 
  foreach(var IN LISTS PACKAGE_LINK_DIRS )
    string( CONCAT PACKAGE_LIBS ${PACKAGE_LIBS} " -L${var}" )
  endforeach()
  foreach(var IN LISTS PACKAGE_LINK_LIBS )
    if( EXISTS ${var} OR  ${var} MATCHES "-framework*" )
      string( CONCAT PACKAGE_LIBS ${PACKAGE_LIBS} " ${var}" )
    else() # assume it's just a -l call??
      string( CONCAT PACKAGE_LIBS ${PACKAGE_LIBS} " -l${var}" )
    endif()
  endforeach()

  # add any CXX flags user has passed in
  if( PACKAGE_CXXFLAGS )
    string( CONCAT PACKAGE_CFLAGS ${PACKAGE_CXXFLAGS} )
  endif()

  if( PACKAGE_LIB_NAME )
    # install library itself
    install( FILES ${_target_library} DESTINATION ${CMAKE_INSTALL_PREFIX}/lib )
    set( PACKAGE_LIB_LINK "-l${PACKAGE_LIB_NAME}" )
  endif()

  # build pkg-config file
  if( PACKAGE_PKG_NAME )
    configure_file( ${modules_dir}/PkgConfig.pc.in ${PACKAGE_PKG_NAME}.pc @ONLY )
    # install pkg-config file for external projects to discover this library
    install( FILES ${CMAKE_CURRENT_BINARY_DIR}/${PACKAGE_PKG_NAME}.pc 
        DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/pkgconfig/ )
  endif()

  if( PACKAGE_INSTALL_HEADERS )
    # install header files
    install( FILES ${PACKAGE_INSTALL_HEADERS} DESTINATION ${PACKAGE_DESTINATION} )
  endif()

  # write and install a cmake "find package" for cmake projects to use.
  # NB: this .camke file CANNOT refer to any source directory, only to
  # _installed_ files.
  configure_file( ${modules_dir}/FindPackage.cmake.in Find${PACKAGE_PKG_NAME}.cmake @ONLY )
  install( FILES ${CMAKE_CURRENT_BINARY_DIR}/Find${PACKAGE_PKG_NAME}.cmake 
      DESTINATION ${CMAKE_INSTALL_PREFIX}/share/${PACKAGE_PKG_NAME}/ )

  #######################################################
  # Export library for easy inclusion from other cmake projects. APPEND allows
  # call to function even as subdirectory of larger project.
  FILE(REMOVE "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake")
  export( TARGETS ${LIBRARY_NAME}
      APPEND FILE "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake" )
#  install( EXPORT ${PROJECT_NAME}Targets DESTINATION ${CMAKECONFIG_INSTALL_DIR} )
#  install(TARGETS ${LIBRARY_NAME}
#      EXPORT ${PROJECT_NAME}Targets DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
#      )

  # Version information.  So find_package( XXX version ) will work.
  configure_file( ${CMAKE_SOURCE_DIR}/cmake_modules/PackageConfigVersion.cmake.in
      "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake" @ONLY )

  # Build tree config.  So some folks can use the built package (e.g., any of our
  # own examples or applcations in this project.
  configure_file( ${CMAKE_SOURCE_DIR}/cmake_modules/PackageConfig.cmake.in
      ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake @ONLY )
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake 
      DESTINATION lib/cmake/${PROJECT_NAME})

  #  # Install tree config.  NB we DO NOT use this.  We install using brew or
  #  set( EXPORT_LIB_INC_DIR ${LIB_INC_DIR} )
  #  pkg-config.
  #  set( EXPORT_LIB_INC_DIR "\${PROJECT_CMAKE_DIR}/${REL_INCLUDE_DIR}" )
  #  configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}Config.cmake.in
  #      ${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/${PROJECT_NAME}Config.cmake @ONLY )

  # Add package to CMake package registery for use from the build tree. RISKY.
  option( EXPORT_${PROJECT_NAME}
      "Should the ${PROJECT_NAME} package be exported for use by other software" OFF )
  if( EXPORT_${PROJECT_NAME} )
    export( PACKAGE ${PROJECT_NAME} )
  endif()


endfunction()

