################################################################################
# export_package.cmake - Functions for easy package exporting	
#
# export_package - Takes a package name and the following optional arguments:
#   - TARGETS: A list of all the targets to export with this package
#   - DEPENDS: A list of all the targets that this package depends 
#              on from outside its directory. 
#   - VERSION: A version string for the package.
#   - INCLUDE_DIRS: The include directories to export.
#   - LINK_DIRS: The link directories where libraries can be found.
#   - LIBRARIES: The libraries to export in ${package}_LIBRARIES.
#   - LIBRARY: A library to export in ${package}_LIBRARY.
#
################################################################################
include(CMakeParseArguments)

get_filename_component(modules_dir ${CMAKE_CURRENT_LIST_FILE} PATH)

function(export_package package)
  string(TOUPPER ${package} PACKAGE)

  set(PACKAGE_OPTIONS)
  set(PACKAGE_SINGLE_ARGS VERSION LIBRARY)
  set(PACKAGE_MULTI_ARGS TARGETS INCLUDE_DIRS LINK_DIRS DEPENDS LIBRARIES)
  cmake_parse_arguments(PACKAGE
    "${PACKAGE_OPTIONS}"
    "${PACKAGE_SINGLE_ARGS}"
    "${PACKAGE_MULTI_ARGS}"
    "${ARGN}"
    )

  set(CMAKECONFIG_INSTALL_DIR "lib/cmake/${package}")
  option(EXPORT_${package}
    "Should the ${package} package be exported for use by other software" OFF)

  # Version information
#  if(PACKAGE_VERSION)
#    configure_file(${modules_dir}/PackageConfigVersion.cmake.in
#      "${CMAKE_CURRENT_BINARY_DIR}/${package}ConfigVersion.cmake" @ONLY)
#    install(FILES
#      "${CMAKE_CURRENT_BINARY_DIR}/${package}ConfigVersion.cmake"
#      DESTINATION ${CMAKECONFIG_INSTALL_DIR} )
#  endif()

  # Build tree config
#  set(EXPORT_PACAKGE_INC PACKAGE_INCLUDE_DIRS)
#  CONFIGURE_FILE(${modules_dir}/PackageConfig.cmake.in
#    ${CMAKE_CURRENT_BINARY_DIR}/${package}Config.cmake @ONLY IMMEDIATE)

  # Install tree config
#  configure_file(${modules_dir}/PackageConfig.cmake.in
#    ${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/${package}Config.cmake @ONLY)

  # Add package to CMake package registery for use from the build tree
  if(EXPORT_${package})
    export(PACKAGE ${package})
  endif()

  file(RELATIVE_PATH REL_INCLUDE_DIR
    "${CMAKE_INSTALL_PREFIX}/${CMAKECONFIG_INSTALL_DIR}"
    "${CMAKE_INSTALL_PREFIX}/include")

  install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/${package}Targets.cmake"
    DESTINATION ${CMAKECONFIG_INSTALL_DIR} )

  if(PACKAGE_TARGETS)
    file(REMOVE "${CMAKE_CURRENT_BINARY_DIR}/${package}Targets.cmake")
    export(TARGETS ${PACKAGE_TARGETS} ${PACKAGE_DEPENDS}
      APPEND FILE "${CMAKE_CURRENT_BINARY_DIR}/${package}Targets.cmake")
  endif()

#  install(EXPORT ${package} DESTINATION ${CMAKECONFIG_INSTALL_DIR})
#  set("${PACKAGE}_DIR" "${CMAKE_CURRENT_BINARY_DIR}" CACHE PATH
#    "Path to ${package} configuration.")

  # Install ${package}Config.cmake, which will read ${package}Targets.cmake
#  install(FILES
#    "${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/${package}Config.cmake"
#    DESTINATION ${CMAKECONFIG_INSTALL_DIR} )

endfunction()

