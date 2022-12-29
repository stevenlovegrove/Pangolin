cmake_minimum_required(VERSION 3.16)

project(Pangolin
  VERSION 2.0
  DESCRIPTION "Lightweight Visualization and Input Library for Computer Vision"
  LANGUAGES CXX
)

# See if we're the root project
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    include(CTest)
endif()

##############################################################################
# Options

option( PANGOLIN_BUILD_TOOLS "Build Tools" ON )
option( PANGOLIN_BUILD_EXAMPLES "Build Examples" ON )
option( PANGOLIN_BUILD_TESTS "Build Tests" ON)
option( PANGOLIN_BUILD_ASAN "Enable AddressSanitizer for Debug builds" OFF )

##############################################################################
# CMake includes / imports

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
include(FetchContent)
include(CMakePackageConfigHelpers)
include(SetPlatformVars)
include(PangolinFactory)

##############################################################################
# Prevent in source builds
if(EXISTS "${PROJECT_BINARY_DIR}/CMakeLists.txt")
    message(FATAL_ERROR "Source build detected: please use a subdir. You may remove 'CMakeCache.txt' and 'CMakeFiles'.")
endif()

##############################################################################
# Default build type (Override with cmake .. -DCMAKE_BUILD_TYPE=...)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build." FORCE)
  message(STATUS "Setting build type to '${CMAKE_BUILD_TYPE}' as none was specified.")
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

##############################################################################
# Declare dependencies. CMake will first try to use find_package for these, so
# you can override location by specifying farm_ng_cmake_DIR, farm_ng_core_DIR,
# Sophus_DIR etc. If they can't be found through find_package, they'll be
# downloaded and configured through cmake. These libs are lightweight.

FetchContent_Declare( farm_ng_cmake
  GIT_REPOSITORY https://github.com/farm-ng/farm-ng-cmake.git
  GIT_TAG        ae21a4f8214b6a3dc248422928f1571cf8387a2e
  FIND_PACKAGE_ARGS NAMES farm_ng_cmake
)
FetchContent_Declare( farm_ng_core
  GIT_REPOSITORY https://github.com/farm-ng/farm-ng-core.git
  GIT_TAG        4225733de17933a44e865e38a65b0061300c88b3
  FIND_PACKAGE_ARGS NAMES farm_ng_core
)
FetchContent_Declare( Sophus
  GIT_REPOSITORY https://github.com/strasdat/Sophus.git
  GIT_TAG        735a2b6d492ceada4d5f744f0f302baae5c17644
  FIND_PACKAGE_ARGS NAMES Sophus
)
FetchContent_Declare( Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        GIT_TAG v2.13.6
  FIND_PACKAGE_ARGS NAMES Catch2
)
FetchContent_Declare( fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt.git
  GIT_TAG 9.1.0
  FIND_PACKAGE_ARGS NAMES fmt
)
FetchContent_MakeAvailable(farm_ng_cmake)
set(farm_ng_core_BUILD_TESTS off)
set(Sophus_BUILD_TESTS off)
set(BUILD_SOPHUS_TESTS off)
include("${farm_ng_cmake_SOURCE_DIR}/cmake/farm_ng_cmakeConfig.cmake")
# set(farm_ng_cmake_DIR "${farm_ng_cmake_SOURCE_DIR}")

##############################################################################
# Setup build config

set(CMAKE_CXX_STANDARD_REQUIRED ON)

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
    set(CMAKE_CXX_STANDARD 20)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wno-unused-parameter -Wno-unused-variable -Wno-unused-function -Wreorder-ctor -Wreorder-init-list")
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -std=gnu++17 -fconcepts -Werror -Wall -Wextra -Wno-unused-parameter -Wno-unused-but-set-variable -Wno-unused-variable -Wno-unused-function -Wno-maybe-uninitialized -Wno-sign-compare -Wno-reorder -Wno-missing-field-initializers")
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

# run with "ASAN_OPTIONS=fast_unwind_on_malloc=0" to print stack with more details
if(PANGOLIN_BUILD_ASAN)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} \
    -fsanitize=address -fsanitize=bool -fsanitize=bounds -fsanitize=enum \
    -fsanitize=float-cast-overflow -fsanitize=float-divide-by-zero \
    -fsanitize=nonnull-attribute -fsanitize=returns-nonnull-attribute \
    -fsanitize=signed-integer-overflow -fsanitize=undefined \
    -fsanitize=vla-bound -fno-sanitize=alignment -fsanitize=leak \
    -fsanitize=object-size")
endif()

#######################################################
## Testing setup

if(PANGOLIN_BUILD_TESTS)
    FetchContent_MakeAvailable(Catch2)
    list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
    include(Catch)
endif()

#######################################################
## Add all pangolin components

FetchContent_MakeAvailable(fmt farm_ng_core Sophus)

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

#######################################################
#

write_basic_package_version_file(
    PangolinConfigVersion.cmake
    VERSION ${CMAKE_PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
    )

#######################################################
## Build Tree Export

# Export Targets
# export(TARGETS ${component_list} FILE PangolinTargets.cmake)
# export(PACKAGE Pangolin)

# # Version information
# configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/cmake/${PROJECT_NAME}ConfigVersion.cmake.in
# "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake" @ONLY)

# Build tree config
# configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/cmake/${PROJECT_NAME}Config.cmake.in
# ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake @ONLY IMMEDIATE )


#######################################################
## Install headers / targets

# This relative path allows installed files to be relocatable.
# set( CMAKECONFIG_INSTALL_DIR lib/cmake/${PROJECT_NAME} )
# file( RELATIVE_PATH REL_INCLUDE_DIR
#     "${CMAKE_INSTALL_PREFIX}/${CMAKECONFIG_INSTALL_DIR}"
#     "${CMAKE_INSTALL_PREFIX}/include"
# )

# install(
#     TARGETS ${component_list}
#     EXPORT ${PROJECT_NAME}Targets
#     LIBRARY DESTINATION lib
#     ARCHIVE DESTINATION lib
#     RUNTIME DESTINATION bin
#     INCLUDES DESTINATION include
# )

# install(
#     FILES "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
#           "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
#     DESTINATION ${CMAKECONFIG_INSTALL_DIR}
# )
# install(
#     EXPORT ${PROJECT_NAME}Targets DESTINATION ${CMAKECONFIG_INSTALL_DIR}
# )

# # uninstall target
# if(NOT TARGET uninstall)
#   configure_file(
#     "${CMAKE_CURRENT_LIST_DIR}/cmake/cmake_uninstall.cmake.in"
#     "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
#     IMMEDIATE @ONLY)

#   add_custom_target(uninstall
#     COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
# endif()


#######################################################
## Tools / Examples

set(Pangolin_DIR ${CMAKE_CURRENT_BINARY_DIR})

if(PANGOLIN_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# if(PANGOLIN_BUILD_TOOLS)
#     add_subdirectory(tools)
# endif()
