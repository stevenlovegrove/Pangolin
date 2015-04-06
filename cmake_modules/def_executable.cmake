include(CMakeParseArguments)
include(SetSourceFlags)

function(def_executable exec)

  string(TOUPPER ${exec} EXEC)

  set(EXEC_OPTIONS)
  set(EXEC_SINGLE_ARGS PACKAGE)
  set(EXEC_MULTI_ARGS SOURCES DEPENDS CONDITIONS LINK_LIBS)
  cmake_parse_arguments(exec
    "${EXEC_OPTIONS}"
    "${EXEC_SINGLE_ARGS}"
    "${EXEC_MULTI_ARGS}"
    "${ARGN}"
    )

  if(NOT exec_SOURCES)
    message(FATAL_ERROR "def_executable for ${EXEC} has an empty source list.")
  endif()

  set(cache_var BUILD_${EXEC})
  set(${cache_var} ON CACHE BOOL "Enable ${EXEC} compilation.")

  set(build_type_cache_var ${EXEC}_BUILD_TYPE)
  set(${build_type_cache_var} "Release" CACHE STRING
    "Target specific build configuration for exec${exec}")

  if(exec_CONDITIONS)
    foreach(cond ${exec_CONDITIONS})
      if(NOT ${cond})
	set(${cache_var} OFF)
	message("${cache_var} is false because ${cond} is false.")
	return()
      endif()
    endforeach()
  endif()

  if(exec_DEPENDS)
    foreach(dep ${exec_DEPENDS})
      if(NOT TARGET ${dep})
	set(${cache_var} OFF)
	message("${cache_var} is false because ${dep} is not being built.")
	return()
      endif()
    endforeach()
  endif()

  if(${cache_var})
    add_executable(${exec} ${exec_SOURCES})

    string(TOUPPER "${${build_type_cache_var}}" EXEC_BUILD_TYPE)

    # Only alter the compile flags if the build type is set
    if(EXEC_BUILD_TYPE)
      foreach(src_file ${exec_SOURCES})
	set_source_flags("${src_file}" "${EXEC_BUILD_TYPE}")
      endforeach()
    endif()

    if(exec_DEPENDS)
      target_link_libraries(${exec} ${exec_DEPENDS})
    endif()

    if(exec_LINK_LIBS)
      target_link_libraries(${exec} ${exec_LINK_LIBS})
    endif()

    if(exec_PACKAGE)
      install(TARGETS ${exec}
	EXPORT ${exec_PACKAGE}
	RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
	LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
	ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
	)
    endif()
  endif()
endfunction()