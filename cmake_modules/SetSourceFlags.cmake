function(set_source_flags src_file build_type)
  get_source_file_property(SRC_LANG "${src_file}" LANGUAGE)

  if("${SRC_LANG}" STREQUAL CXX)
    set(src_file_flags "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_${build_type}}")
  elseif("${SRC_LANG}" STREQUAL C)
    set(src_file_flags "${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_${build_type}}")
  else()
    return()
  endif()

  set_source_files_properties("${src_file}" PROPERTIES COMPILE_FLAGS "${src_file_flags}")
endfunction()