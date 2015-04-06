include(CMakeParseArguments)
include(SetSourceFlags)

function(def_library lib)
  
  string(TOUPPER ${lib} LIB)

  set(LIB_OPTIONS)
  set(LIB_SINGLE_ARGS PACKAGE)
  set(LIB_MULTI_ARGS SOURCES INSTALL_FILES DEPENDS CONDITIONS LINK_LIBS)
  cmake_parse_arguments(lib
    "${LIB_OPTIONS}"
    "${LIB_SINGLE_ARGS}"
    "${LIB_MULTI_ARGS}"
    "${ARGN}"
    )

#  message(STATUS "The following files will be installed: ${lib_INSTALL_FILES}")
#  message(STATUS "sources: ${lib_SOURCES}")
 
  if(NOT lib_SOURCES)
    message(FATAL_ERROR "def_library for ${LIB} has an empty source list.")
  endif()

  set(cache_var BUILD_${LIB})
  set(${cache_var} ON CACHE BOOL "Enable ${LIB} compilation.")

  set(build_type_cache_var LIB${LIB}_BUILD_TYPE)
  set(${build_type_cache_var} "" CACHE STRING
    "Target specific build configuration for lib${lib}")

  if(lib_CONDITIONS)
    foreach(cond ${lib_CONDITIONS})
      if(NOT ${cond})
	set(${cache_var} OFF)
	message("${cache_var} is false because ${cond} is false.")
	return()
      endif()
    endforeach()
  endif()

  if(lib_DEPENDS)
    foreach(dep ${lib_DEPENDS})
      if(NOT TARGET ${dep})
	set(${cache_var} OFF)
	message("${cache_var} is false because ${dep} is not being built.")
	return()
      endif()
    endforeach()
  endif()

  if(${cache_var})
    if(ANDROID)
      add_library(${lib} SHARED ${lib_SOURCES})
    else()
      add_library(${lib} ${lib_SOURCES})
    endif()
    string(TOUPPER "${${build_type_cache_var}}" LIB_BUILD_TYPE)

    # Only alter the compile flags if the build type is set
    if (LIB_BUILD_TYPE)
      foreach(src_file ${lib_SOURCES})
	set_source_flags("${src_file}" "${LIB_BUILD_TYPE}")
      endforeach()
    endif()

    if(lib_DEPENDS)
      target_link_libraries(${lib} PRIVATE ${lib_DEPENDS})
    endif()

    if(lib_LINK_LIBS)
      target_link_libraries(${lib} PRIVATE ${lib_LINK_LIBS})
    endif()

    if(ANDROID)
      find_library(GNUSTL_SHARED_LIBRARY gnustl_shared)

      if(NOT GNUSTL_SHARED_LIBRARY)
	message(FATAL_ERROR "Could not find required GNU STL shared library.")
      endif()
      target_link_libraries(${lib} PRIVATE log android z -pthread ${GNUSTL_SHARED_LIBRARY})
    endif()

    if(lib_PACKAGE)
      install(TARGETS ${lib}
	EXPORT ${lib_PACKAGE}
	RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
	LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
	ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
	)
      install( FILES ${lib_INSTALL_FILES}
          DESTINATION ${CMAKE_INSTALL_PREFIX}/include/Pangolin )
  endif()
  endif()
endfunction()
