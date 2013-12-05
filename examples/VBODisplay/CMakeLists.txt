# Find Pangolin (https://github.com/stevenlovegrove/Pangolin)
find_package(Pangolin 0.1 REQUIRED)
include_directories(${Pangolin_INCLUDE_DIRS})
link_directories(${Pangolin_LIBRARY_DIRS})
link_libraries(${Pangolin_LIBRARIES})

find_package(CUDA QUIET)

# This example could be made to work with C++11, but the kernel code must be
# compiled without it.
if(CUDA_FOUND AND NOT CPP11_NO_BOOST)
  cuda_include_directories(${CMAKE_CURRENT_SOURCE_DIR})

  set(CUDA_NVCC_FLAGS "-O3")
  add_definitions(-DCUDA_BUILD)

  if(CUDA_SDK_ROOT_DIR AND EXISTS("${CUDA_SDK_ROOT_DIR}/lib/libcutil_x86_64.a") )
    add_definitions(-DUSE_CUTIL)
    include_directories( ${CUDA_SDK_ROOT_DIR}/common/inc )
    link_directories( ${CUDA_SDK_ROOT_DIR}/lib )
  endif()

  cuda_add_executable(
    VBODisplay
    main.cpp kernal.cu
  )

  if(CUDA_SDK_ROOT_DIR AND EXISTS("${CUDA_SDK_ROOT_DIR}/lib/libcutil_x86_64.a") )
    target_link_libraries(VBODisplay libcutil_x86_64.a)
  endif()
endif()
