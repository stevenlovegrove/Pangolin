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
      INSTALL_GENERATED_HEADERS 
      INSTALL_HEADER_DIRS
      INSTALL_INCLUDE_DIR
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

  # Add package to CMake package registery for use from the build tree. RISKY.
  option( EXPORT_${PROJECT_NAME}
      "Should the ${PROJECT_NAME} package be exported for use by other software" OFF )

  mark_as_advanced( EXPORT_${PROJECT_NAME} )


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

  # construct Cflags arguments for pkg-config file
  set( PACKAGE_CFLAGS "${PACKAGE_CFLAGS} ${CMAKE_C_FLAGS}" )
  foreach(var IN LISTS PACKAGE_INCLUDE_DIRS )
    set( PACKAGE_CFLAGS "${PACKAGE_CFLAGS} -I${var}" )
  endforeach()

  # now construct Libs.private arguments 
  foreach(var IN LISTS PACKAGE_LINK_DIRS )
    set( PACKAGE_LIBS "${PACKAGE_LIBS} -L${var}" )
  endforeach()
  foreach(var IN LISTS PACKAGE_LINK_LIBS )
    if( EXISTS ${var} OR  ${var} MATCHES "-framework*" )
      set( PACKAGE_LIBS "${PACKAGE_LIBS} ${var}" )
    else() # assume it's just a -l call??
      set( PACKAGE_LIBS "${PACKAGE_LIBS} -l${var}" )
    endif()
  endforeach()

  # add any CXX flags user has passed in
  if( PACKAGE_CXXFLAGS )
    set( PACKAGE_CFLAGS ${PACKAGE_CXXFLAGS} )
  endif()

  # In case we want to install. 
  if( NOT EXPORT_${PROJECT_NAME} )
        # add "installed" library to list of required libraries to link against
        if( PACKAGE_LIB_NAME )
            if(POLICY CMP0026)
              cmake_policy( SET CMP0026 OLD )
            endif()
            get_target_property( _target_library ${PACKAGE_LIB_NAME} LOCATION )
            get_filename_component( _lib ${_target_library} NAME )
            list( APPEND PACKAGE_LINK_LIBS ${PACKAGE_LIB_NAME} )
        endif()

        if( PACKAGE_INSTALL_HEADER_DIRS )
            foreach(dir IN LISTS PACKAGE_INSTALL_HEADER_DIRS )
            install( DIRECTORY ${dir}
                DESTINATION ${PACKAGE_DESTINATION}/include 
                FILES_MATCHING PATTERN "*.h|*.hxx|*.hpp"
                )
            endforeach()
        endif()

        # install header files
        if( PACKAGE_INSTALL_HEADERS )
#            install( FILES ${PACKAGE_INSTALL_HEADERS} DESTINATION ${PACKAGE_DESTINATION} )
          foreach(hdr IN LISTS PACKAGE_INSTALL_HEADERS )
             get_filename_component( _fp ${hdr} ABSOLUTE )
             file( RELATIVE_PATH _rpath ${CMAKE_BINARY_DIR} ${_fp} )
             get_filename_component( _dir ${_rpath} DIRECTORY )
             install( FILES ${_fp}
                 DESTINATION ${PACKAGE_DESTINATION}/${_dir} )
         endforeach()
        endif()
        if( PACKAGE_INSTALL_GENERATED_HEADERS )
          foreach(hdr IN LISTS PACKAGE_INSTALL_GENERATED_HEADERS )
             get_filename_component( _fp ${hdr} ABSOLUTE )
             file( RELATIVE_PATH _rpath ${CMAKE_BINARY_DIR} ${_fp} )
             get_filename_component( _dir ${_rpath} DIRECTORY )
             install( FILES ${_fp}
                 DESTINATION ${PACKAGE_DESTINATION}/${_dir} )
         endforeach()
        endif()

        if( PACKAGE_INSTALL_INCLUDE_DIR )
            install(DIRECTORY ${CMAKE_SOURCE_DIR}/include DESTINATION ${PACKAGE_DESTINATION} )
        endif()

        # install library itself
        if( PACKAGE_LIB_NAME )
            install( FILES ${_target_library} DESTINATION ${CMAKE_INSTALL_PREFIX}/lib )
            set( PACKAGE_LIB_LINK "-l${PACKAGE_LIB_NAME}" )
        endif()
    
        # build pkg-config file
        if( PACKAGE_PKG_NAME )
            configure_file( ${modules_dir}/PkgConfig.pc.in ${PACKAGE_PKG_NAME}.pc @ONLY )
        
        # install pkg-config file for external projects to discover this library
            install( FILES ${CMAKE_CURRENT_BINARY_DIR}/${PACKAGE_PKG_NAME}.pc 
                DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/pkgconfig/ )
        
        #######################################################
        # Export library for easy inclusion from other cmake projects. APPEND allows
        # call to function even as subdirectory of larger project.
        FILE(REMOVE "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake")
        export( TARGETS ${LIBRARY_NAME}
        APPEND FILE "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake" )

        # Version information.  So find_package( XXX version ) will work.
        configure_file( ${CMAKE_SOURCE_DIR}/cmake_modules/PackageConfigVersion.cmake.in
        "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake" @ONLY )

        # Build tree config.  So some folks can use the built package (e.g., any of our
        # own examples or applcations in this project.
        configure_file( ${CMAKE_SOURCE_DIR}/cmake_modules/PackageConfig.cmake.in
            ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake @ONLY )
        install(FILES
            ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
            ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake
            ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
            DESTINATION
            lib/cmake/${PROJECT_NAME})

        install( FILES ${CMAKE_CURRENT_BINARY_DIR}/Find${PACKAGE_PKG_NAME}.cmake 
            DESTINATION ${CMAKE_INSTALL_PREFIX}/share/${PACKAGE_PKG_NAME}/ )


  #  # Install tree config.  NB we DO NOT use this.  We install using brew or
  #  pkg-config.
  #  set( EXPORT_LIB_INC_DIR ${LIB_INC_DIR} )
  #  set( EXPORT_LIB_INC_DIR "\${PROJECT_CMAKE_DIR}/${REL_INCLUDE_DIR}" )
  #  configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}Config.cmake.in
  #      ${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/${PROJECT_NAME}Config.cmake @ONLY )
        endif()

  # In case we want to export.
  elseif( EXPORT_${PROJECT_NAME} )

      if( PACKAGE_LIB_NAME )
            if(POLICY CMP0026)
              cmake_policy( SET CMP0026 OLD )
            endif()
            get_target_property( _target_library ${PACKAGE_LIB_NAME} LOCATION )
            list( APPEND PACKAGE_LINK_LIBS ${_target_library} )
        endif()

        if( PACKAGE_INSTALL_HEADER_DIRS )
            foreach(dir IN LISTS PACKAGE_INSTALL_HEADER_DIRS )
                list( APPEND PACKAGE_INCLUDE_DIRS ${dir} )
            endforeach()
        endif()

        #if( PACKAGE_INSTALL_HEADER_DIRS )
        #   foreach(dir IN LISTS PACKAGE_INSTALL_HEADER_DIRS )
        #        FILE( GLOB ${dir} "*.h" "*.hpp" )
        #        list( APPEND PACKAGE_INCLUDE_DIRS ${dir} )
        #    endforeach()
        #endif()

        list( APPEND PACKAGE_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/include
            ${CMAKE_CURRENT_BINARY_DIR}/include )

        # install library itself
        #if( PACKAGE_LIB_NAME )
        #    set( PACKAGE_LIB_LINK "-l${PACKAGE_LIB_NAME}" )
        #endif()
   #######################################################
  # Export library for easy inclusion from other cmake projects. APPEND allows
  # call to function even as subdirectory of larger project.
  FILE(REMOVE "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake")
  export( TARGETS ${LIBRARY_NAME}
      APPEND FILE "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake" )

  export( PACKAGE ${PROJECT_NAME} )
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
  
  #  # Install tree config.  NB we DO NOT use this.  We install using brew or
  #  pkg-config.
  #  set( EXPORT_LIB_INC_DIR ${LIB_INC_DIR} )
  #  set( EXPORT_LIB_INC_DIR "\${PROJECT_CMAKE_DIR}/${REL_INCLUDE_DIR}" )
  #  configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}Config.cmake.in
  #      ${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/${PROJECT_NAME}Config.cmake @ONLY )
    #export( PACKAGE ${PROJECT_NAME} )
  endif()


  # write and install a cmake "find package" for cmake projects to use.
  # NB: this .cmake file CANNOT refer to any source directory, only to
  # _installed_ files.
  configure_file( ${modules_dir}/FindPackage.cmake.in Find${PACKAGE_PKG_NAME}.cmake @ONLY )
    
endfunction()

