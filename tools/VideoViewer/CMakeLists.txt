# Find Pangolin (https://github.com/stevenlovegrove/Pangolin)
find_package(Pangolin 0.2 REQUIRED)
include_directories(${Pangolin_INCLUDE_DIRS})

add_executable(VideoViewer main.cpp)
target_link_libraries(VideoViewer ${Pangolin_LIBRARIES})


# Make file association
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    file( GENERATE
        OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/pango.desktop"
        CONTENT
"[Desktop Entry]
Name=PangoVideoViewer
Exec=$<TARGET_FILE:VideoViewer>
MimeType=application/x-pango
Icon=
Terminal=false
Type=Application
Categories=AudioVideo;Player;Video;
Comment="
    )

    add_custom_target(
        install-mime-info
        COMMAND mkdir -p $ENV{HOME}/.local/share/mime/packages/
        COMMAND mkdir -p $ENV{HOME}/.local/share/applications/
        COMMAND cp "${CMAKE_CURRENT_SOURCE_DIR}/application-x-pango.xml" $ENV{HOME}/.local/share/mime/packages/
        COMMAND cp "${CMAKE_CURRENT_BINARY_DIR}/pango.desktop"           $ENV{HOME}/.local/share/applications/
        COMMAND update-mime-database    $ENV{HOME}/.local/share/mime
        COMMAND update-desktop-database $ENV{HOME}/.local/share/applications
        DEPENDS VideoViewer
        VERBATIM
    )
endif()
