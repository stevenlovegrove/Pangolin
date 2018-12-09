#
# While not strictly necessary to specify header files as target sources,
# doing so populates these files in the source listing when CMake is used
# to generate XCode and Visual Studios projects
#
target_sources(mio_base INTERFACE
  $<BUILD_INTERFACE:
    ${CMAKE_CURRENT_LIST_DIR}/mmap.hpp
    ${CMAKE_CURRENT_LIST_DIR}/page.hpp
    ${CMAKE_CURRENT_LIST_DIR}/shared_mmap.hpp>
  $<INSTALL_INTERFACE:
    include/mio/mmap.hpp
    include/mio/page.hpp
    include/mio/shared_mmap.hpp>)

add_subdirectory(detail)
