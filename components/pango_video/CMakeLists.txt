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

PangolinRegisterFactory(
    VideoInterface
    TestVideo ImagesVideo SplitVideo TruncateVideo PangoVideo
    DebayerVideo ShiftVideo TransformVideo UnpackVideo PackVideo
    JoinVideo MergeVideo JsonVideo
)

PangolinRegisterFactory(
    VideoOutputInterface
    ImagesVideoOutput PangoVideoOutput
)

target_include_directories(${COMPONENT}
    PUBLIC "${CMAKE_CURRENT_LIST_DIR}/include"
    PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/include"
)


target_link_libraries(${COMPONENT} PUBLIC pango_core pango_image pango_packetstream)
