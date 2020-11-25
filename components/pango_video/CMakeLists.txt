get_filename_component(COMPONENT ${CMAKE_CURRENT_LIST_DIR} NAME)

target_sources( ${COMPONENT}
PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/src/stream_encoder_factory.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/video_input.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/video_output.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/video.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/help.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/drivers/test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/drivers/images.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/drivers/images_out.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/drivers/split.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/drivers/truncate.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/drivers/pango.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/drivers/pango_video_output.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/drivers/debayer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/drivers/shift.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/drivers/transform.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/drivers/unpack.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/drivers/pack.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/drivers/join.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/drivers/merge.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/drivers/json.cpp
)

list(APPEND VIDEO_FACTORY_REG
    RegisterTestVideoFactory
    RegisterImagesVideoFactory
    RegisterImagesVideoOutputFactory
    RegisterSplitVideoFactory
    RegisterTruncateVideoFactory
    RegisterPangoVideoFactory
    RegisterPangoVideoOutputFactory
    RegisterDebayerVideoFactory
    RegisterShiftVideoFactory
    RegisterTransformVideoFactory
    RegisterUnpackVideoFactory
    RegisterPackVideoFactory
    RegisterJoinVideoFactory
    RegisterMergeVideoFactory
    RegisterJsonVideoFactory
)

target_include_directories(${COMPONENT}
    PUBLIC "${CMAKE_CURRENT_LIST_DIR}/include"
    PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/include"
)


target_link_libraries(${COMPONENT} PUBLIC pango_core pango_image pango_packetstream)

# Create window_frameworks.h file for inclusion in library
# This loads windowing framework factories based on cmake configuration
CreateMethodCallFile(
    "${CMAKE_CURRENT_BINARY_DIR}/include/pangolin/video/video_drivers.h"
    "pangolin" "LoadBuiltInVideoDrivers" "${VIDEO_FACTORY_REG}"
)
