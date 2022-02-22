get_filename_component(COMPONENT ${CMAKE_CURRENT_LIST_DIR} NAME)

set(DRIVER_DIR "${CMAKE_CURRENT_LIST_DIR}/src/drivers")

target_sources( ${COMPONENT}
PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/src/stream_encoder_factory.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/video_input.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/video_output.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/video.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/video_help.cpp
    ${DRIVER_DIR}/test.cpp
    ${DRIVER_DIR}/images.cpp
    ${DRIVER_DIR}/images_out.cpp
    ${DRIVER_DIR}/split.cpp
    ${DRIVER_DIR}/truncate.cpp
    ${DRIVER_DIR}/pango.cpp
    ${DRIVER_DIR}/pango_video_output.cpp
    ${DRIVER_DIR}/debayer.cpp
    ${DRIVER_DIR}/shift.cpp
    ${DRIVER_DIR}/transform.cpp
    ${DRIVER_DIR}/unpack.cpp
    ${DRIVER_DIR}/pack.cpp
    ${DRIVER_DIR}/join.cpp
    ${DRIVER_DIR}/merge.cpp
    ${DRIVER_DIR}/json.cpp
    ${DRIVER_DIR}/thread.cpp
    ${DRIVER_DIR}/mjpeg.cpp
)

PangolinRegisterFactory(
    VideoInterface
    TestVideo ImagesVideo SplitVideo TruncateVideo PangoVideo
    DebayerVideo ShiftVideo TransformVideo UnpackVideo PackVideo
    JoinVideo MergeVideo JsonVideo MjpegVideo
)

PangolinRegisterFactory(
    VideoOutputInterface
    ImagesVideoOutput PangoVideoOutput
)



#########################################################
# Search for third-party libraries

if (UNIX)
    target_sources( ${COMPONENT} PRIVATE ${DRIVER_DIR}/shared_memory.cpp )
    PangolinRegisterFactory( VideoInterface ThreadVideo )
endif()

option(BUILD_PANGOLIN_LIBDC1394 "Build support for libdc1394 video input" ON)
if(BUILD_PANGOLIN_LIBDC1394)
  find_package(DC1394 QUIET)
  if(DC1394_FOUND)
    target_sources( ${COMPONENT} PRIVATE ${DRIVER_DIR}/firewire.cpp ${DRIVER_DIR}/deinterlace.cpp)
    target_link_libraries(${COMPONENT} PRIVATE ${DC1394_LIBRARY} )
    target_include_directories(${COMPONENT} PRIVATE ${DC1394_INCLUDE_DIR} )
    PangolinRegisterFactory( VideoInterface   FirewireVideo DeinterlaceVideo)
    message(STATUS "libdc1394 Found and Enabled")
  endif()
endif()

option(BUILD_PANGOLIN_V4L "Build support for V4L video input" ON)
if(BUILD_PANGOLIN_V4L AND _LINUX_)
    target_sources( ${COMPONENT} PRIVATE ${DRIVER_DIR}/v4l.cpp)
    PangolinRegisterFactory( VideoInterface V4lVideo )
    message(STATUS "V4L Found and Enabled")
endif()

option(BUILD_PANGOLIN_FFMPEG "Build support for ffmpeg video input" ON)
if(BUILD_PANGOLIN_FFMPEG)
  find_package(FFMPEG QUIET)
  if(FFMPEG_FOUND)
      target_sources( ${COMPONENT} PRIVATE
          ${DRIVER_DIR}/ffmpeg.cpp
          ${DRIVER_DIR}/ffmpeg_convert.cpp
          ${DRIVER_DIR}/ffmpeg_output.cpp
      )
      target_link_libraries(${COMPONENT} PRIVATE ${FFMPEG_LIBRARIES} )
      target_include_directories(${COMPONENT} PRIVATE ${FFMPEG_INCLUDE_DIRS} )
      PangolinRegisterFactory( VideoInterface FfmpegVideo)
      PangolinRegisterFactory( VideoInterface FfmpegVideo FfmpegVideoConvert)
      PangolinRegisterFactory( VideoOutputInterface FfmpegVideoOutput)
      if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        # FFMPEG is a real pain for deprecating the API.
        set_source_files_properties(${DRIVER_DIR}/ffmpeg.cpp  PROPERTIES COMPILE_FLAGS "-Wno-deprecated-declarations")
      endif()
      message(STATUS "ffmpeg Found and Enabled: ${FFMPEG_INCLUDE_DIRS}")
  endif()
endif()

option(BUILD_PANGOLIN_REALSENSE "Build support for RealSense video input" ON)
if(BUILD_PANGOLIN_REALSENSE)
  find_package(RealSense QUIET)
  if(REALSENSE_FOUND)
      target_sources( ${COMPONENT} PRIVATE ${DRIVER_DIR}/realsense.cpp)
      target_link_libraries(${COMPONENT} PRIVATE ${REALSENSE_LIBRARIES} )
      target_include_directories(${COMPONENT} PRIVATE ${REALSENSE_INCLUDE_DIRS} )
      PangolinRegisterFactory( VideoInterface RealSenseVideo )
      message(STATUS "RealSense Found and Enabled")
  endif()
endif()

option(BUILD_PANGOLIN_REALSENSE2 "Build support for RealSense2 video input" ON)
if(BUILD_PANGOLIN_REALSENSE2)
  find_package(RealSense2 QUIET)
  if(REALSENSE2_FOUND)
      target_sources( ${COMPONENT} PRIVATE ${DRIVER_DIR}/realsense2.cpp)
      target_link_libraries(${COMPONENT} PRIVATE ${REALSENSE2_LIBRARIES} )
      target_include_directories(${COMPONENT} PRIVATE ${REALSENSE2_INCLUDE_DIRS} )
      PangolinRegisterFactory( VideoInterface RealSense2Video )
      message(STATUS "RealSense2 Found and Enabled")
  endif()
endif()

option(BUILD_PANGOLIN_OPENNI "Build support for OpenNI video input" ON)
if(BUILD_PANGOLIN_OPENNI)
  find_package(OpenNI QUIET)
  if(OPENNI_FOUND)
    target_sources( ${COMPONENT} PRIVATE ${DRIVER_DIR}/openni.cpp)
    target_link_libraries(${COMPONENT} PRIVATE ${OPENNI_LIBRARIES} )
    target_include_directories(${COMPONENT} PRIVATE ${OPENNI_INCLUDE_DIRS} )
    PangolinRegisterFactory( VideoInterface OpenNiVideo )
    message(STATUS "OpenNI Found and Enabled")
  endif()
endif()

option(BUILD_PANGOLIN_OPENNI2 "Build support for OpenNI2 video input" ON)
if(BUILD_PANGOLIN_OPENNI2)
  find_package(OpenNI2 QUIET)
  if(OPENNI2_FOUND)
      if(LINUX)
          target_compile_definitions(${COMPONENT} PRIVATE linux)
      endif()

    target_sources( ${COMPONENT} PRIVATE ${DRIVER_DIR}/openni2.cpp)
    target_link_libraries(${COMPONENT} PRIVATE ${OPENNI2_LIBRARIES} )
    target_include_directories(${COMPONENT} PRIVATE ${OPENNI2_INCLUDE_DIRS} )
    PangolinRegisterFactory( VideoInterface OpenNi2Video )
    message(STATUS "OpenNI2 Found and Enabled")
  endif()
endif()

option(BUILD_PANGOLIN_LIBUVC "Build support for libuvc video input" ON)
if(BUILD_PANGOLIN_LIBUVC)
  find_package(uvc QUIET)
  if(uvc_FOUND)
      target_sources( ${COMPONENT} PRIVATE ${DRIVER_DIR}/uvc.cpp)
      target_link_libraries(${COMPONENT} PRIVATE ${uvc_LIBRARIES} )
      target_include_directories(${COMPONENT} PRIVATE ${uvc_INCLUDE_DIRS} )
      PangolinRegisterFactory( VideoInterface UvcVideo )
      if(WIN32 OR WIN64)
          find_package(pthread REQUIRED QUIET)
          find_package(libusb1 REQUIRED QUIET)
          target_link_libraries(${COMPONENT} PRIVATE
              ${pthread_LIBRARIES} ${libusb1_LIBRARIES}
          )
      endif()
      message(STATUS "libuvc Found and Enabled")
  endif()
endif()

option(BUILD_PANGOLIN_UVC_MEDIAFOUNDATION "Build support for MediaFoundation UVC input" ON)
if (BUILD_PANGOLIN_UVC_MEDIAFOUNDATION)
  find_package(MediaFoundation QUIET)
  if (MediaFoundation_FOUND)
      target_sources( ${COMPONENT} PRIVATE ${DRIVER_DIR}/uvc_mediafoundation.cpp)
      target_link_libraries(${COMPONENT} PRIVATE ${MediaFoundation_LIBRARIES} )
      PangolinRegisterFactory( VideoInterface UvcMediaFoundationVideo )
      message(STATUS "MediaFoundation Found and Enabled")
  endif()
endif()

option(BUILD_PANGOLIN_DEPTHSENSE "Build support for DepthSense video input" ON)
if(BUILD_PANGOLIN_DEPTHSENSE)
  find_package(DepthSense QUIET)
  if(DepthSense_FOUND)
      target_sources( ${COMPONENT} PRIVATE ${DRIVER_DIR}/depthsense.cpp)
      target_link_libraries(${COMPONENT} PRIVATE ${DepthSense_LIBRARIES} )
      target_include_directories(${COMPONENT} PRIVATE ${DepthSense_INCLUDE_DIRS} )
      PangolinRegisterFactory( VideoInterface DepthSenseVideo )
      message(STATUS "DepthSense Found and Enabled")
  endif()
endif()

option(BUILD_PANGOLIN_TELICAM "Build support for TeliCam video input" ON)
if(BUILD_PANGOLIN_TELICAM)
  find_package(TeliCam QUIET)
  if(TeliCam_FOUND)
      target_sources( ${COMPONENT} PRIVATE ${DRIVER_DIR}/teli.cpp)
      target_link_libraries(${COMPONENT} PRIVATE ${TeliCam_LIBRARIES} )
      target_include_directories(${COMPONENT} PRIVATE ${TeliCam_INCLUDE_DIRS} )
      PangolinRegisterFactory( VideoInterface TeliVideo )
      message(STATUS "TeliCam Found and Enabled" )
  endif()
endif()

option(BUILD_PANGOLIN_PLEORA "Build support for Pleora video input" ON)
if(BUILD_PANGOLIN_PLEORA)
  find_package(Pleora QUIET)
  if(Pleora_FOUND)
      target_sources( ${COMPONENT} PRIVATE ${DRIVER_DIR}/pleora.cpp)
      target_link_libraries(${COMPONENT} PRIVATE ${Pleora_LIBRARIES} )
      target_include_directories(${COMPONENT} PRIVATE ${Pleora_INCLUDE_DIRS} )
      PangolinRegisterFactory( VideoInterface PleoraVideo )
      if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
          # Suppress warnings generated from Pleora SDK.
          set_source_files_properties(${DRIVER_DIR}/pleora.cpp PROPERTIES COMPILE_FLAGS -Wno-unknown-pragmas)
      endif()
      message(STATUS "Pleora Found and Enabled" )
  endif()
endif()

#########################################################

target_link_libraries(${COMPONENT} PUBLIC pango_core pango_image pango_packetstream)
target_include_directories(${COMPONENT} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
install(DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/include" DESTINATION ${CMAKE_INSTALL_PREFIX})
install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/include" DESTINATION ${CMAKE_INSTALL_PREFIX})

# Create RegisterFactoriesVideoInterface() method call file to load built drivers and add as source
create_factory_registry_file( "${CMAKE_CURRENT_BINARY_DIR}/include/pangolin/factory" VideoInterface )
create_factory_registry_file( "${CMAKE_CURRENT_BINARY_DIR}/include/pangolin/factory" VideoOutputInterface )
target_sources(${COMPONENT} PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/include/pangolin/factory/RegisterFactoriesVideoInterface.h")
target_sources(${COMPONENT} PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/include/pangolin/factory/RegisterFactoriesVideoOutputInterface.h")


if(BUILD_TESTS)
    add_executable(test_video_uris ${CMAKE_CURRENT_LIST_DIR}/tests/tests_video_uri.cpp)
    target_link_libraries(test_video_uris PRIVATE Catch2::Catch2 ${COMPONENT})
    catch_discover_tests(test_video_uris)
    add_executable(test_video_loading ${CMAKE_CURRENT_LIST_DIR}/tests/tests_video_loading.cpp)
    target_link_libraries(test_video_loading PRIVATE Catch2::Catch2 ${COMPONENT})
    catch_discover_tests(test_video_loading)
endif()
