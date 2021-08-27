cmake_minimum_required(VERSION 3.8)

project("Pangolin")
set(PANGOLIN_VERSION_MAJOR 0)
set(PANGOLIN_VERSION_MINOR 7)
set(PANGOLIN_VERSION ${PANGOLIN_VERSION_MAJOR}.${PANGOLIN_VERSION_MINOR})

if(NOT CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()

# Prevent in source builds
if(EXISTS "${PROJECT_BINARY_DIR}/CMakeLists.txt")
    message(FATAL_ERROR "Source build detected: please use a subdir. You may remove 'CMakeCache.txt' and 'CMakeFiles'.")
endif()

# Make our own cmake imports accessible
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake" ${CMAKE_MODULE_PATH})

include(CreateMethodCallFile)
include(SetPlatformVars)
include(PangolinFactory)

option( BUILD_TOOLS "Build Tools" ON )
option( BUILD_EXAMPLES "Build Examples" ON )

# Default build type (Override with cmake .. -DCMAKE_BUILD_TYPE=...)
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build." FORCE)
  message(STATUS "Setting build type to '${CMAKE_BUILD_TYPE}' as none was specified.")
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

if(_WIN_)
    option( BUILD_SHARED_LIBS    "Build Shared Library" OFF)
    option( MSVC_USE_STATIC_CRT  "Use static C Runtime with MSVC, /MT instead of /MD" ON)

    # Make sure there are no erroneous C Runtime flags
    list(APPEND FLAG_VARS
        CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO
        CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE CMAKE_C_FLAGS_MINSIZEREL CMAKE_C_FLAGS_RELWITHDEBINFO
    )
    if(MSVC_USE_STATIC_CRT)
        foreach(FLAG_VAR ${FLAG_VARS})
            string(REGEX REPLACE "/MD" "/MT" NEW_FLAGS "${${FLAG_VAR}}")
            set(${FLAG_VAR} "${NEW_FLAGS}" CACHE STRING "" FORCE)
        endforeach()
    else()
        foreach(FLAG_VAR ${FLAG_VARS})
            string(REGEX REPLACE "/MT" "/MD" NEW_FLAGS "${${FLAG_VAR}}")
            set(${FLAG_VAR} "${NEW_FLAGS}" CACHE STRING "" FORCE)
        endforeach()
    endif()
elseif(_OSX_)
    option( BUILD_SHARED_LIBS "Build Shared Library" ON)
    set(CMAKE_MACOSX_RPATH ON)
elseif(EMSCRIPTEN)
    set(BUILD_SHARED_LIBS OFF)
    include(EmscriptenUtils)
    set(CMAKE_EXE_LINKER_FLAGS "-sASYNCIFY=1 -sDISABLE_EXCEPTION_CATCHING=0 -sGL_ASSERTIONS=1 -sFULL_ES2=1 -sFULL_ES3=1 --bind")
else()
    option( BUILD_SHARED_LIBS "Build Shared Library" ON)
endif()

# Add all pangolin components
file(GLOB components_cmake "${CMAKE_CURRENT_LIST_DIR}/components/*/CMakeLists.txt")
set(component_list "")
foreach(component_cmake ${components_cmake})
    get_filename_component(component_dir ${component_cmake} DIRECTORY)
    get_filename_component(component_name ${component_dir} NAME)
    add_library(${component_name} "")
    list(APPEND component_list ${component_name})
endforeach()
foreach(component_cmake ${components_cmake})
    include(${component_cmake})
endforeach()

# Create Factory Registry files to force symbol loading
PangolinCreateFactoryRegisterFile( "${CMAKE_CURRENT_BINARY_DIR}/include/pangolin/factory" pangolin)

# Export Targets
export(TARGETS ${component_list} FILE PangolinTargets.cmake)
export(PACKAGE Pangolin)

# Version information
configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/cmake/${PROJECT_NAME}ConfigVersion.cmake.in
  "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake" @ONLY)

# Build tree config
configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/cmake/${PROJECT_NAME}Config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake @ONLY IMMEDIATE )

set(Pangolin_DIR ${CMAKE_CURRENT_BINARY_DIR})

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

if(BUILD_TOOLS)
    add_subdirectory(tools)
endif()
