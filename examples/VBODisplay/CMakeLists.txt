# Find Pangolin (https://github.com/stevenlovegrove/Pangolin)
include_directories(${PANGOLIN_INCLUDE_DIRS})
link_directories(${PANGOLIN_LIBRARY_DIRS})
link_libraries(${PANGOLIN_LIBRARIES})

find_package(CUDA QUIET)

# This example could be made to work with C++11, but the kernel code must be
# compiled without it.
if(CUDA_FOUND)
  cuda_include_directories(${CMAKE_CURRENT_SOURCE_DIR})

  cuda_add_executable(
    VBODisplay
    main.cpp kernal.cu
  )

endif()
