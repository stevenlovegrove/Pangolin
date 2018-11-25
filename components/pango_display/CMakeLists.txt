get_filename_component(COMPONENT ${CMAKE_CURRENT_LIST_DIR} NAME)

target_sources( ${COMPONENT}
PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/src/display.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/window_factory.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/image_view.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/view.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/widgets.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/handler.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/handler_image.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/handler_glbuffer.cpp
)

set( WINDOW_FACTORY_REG "" )

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    list(APPEND WINDOW_FACTORY_REG RegisterOsxWindowFactory)
    target_link_libraries(${COMPONENT} PRIVATE "-framework Cocoa" )
    target_sources( ${COMPONENT}
        PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/src/device/display_osx.mm
        ${CMAKE_CURRENT_LIST_DIR}/src/device/PangolinNSApplication.mm
        ${CMAKE_CURRENT_LIST_DIR}/src/device/PangolinNSGLView.mm
    )
    target_compile_definitions(${COMPONENT} PUBLIC "PANGO_DEFAULT_WIN_URI=\"cocoa\"")
    target_compile_options(${COMPONENT} PRIVATE
        $<$<OR:$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:GNU>>:
        -Wno-deprecated-declarations>
    )
elseif(WIN32 OR WIN64)
    list(APPEND WINDOW_FACTORY_REG RegisterWinWindowFactory)
    target_sources( ${COMPONENT} PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src/device/display_win.cpp )
    target_compile_definitions(${COMPONENT} PUBLIC "PANGO_DEFAULT_WIN_URI=\"winapi\"")
else()
    find_package(X11 QUIET)
    if(X11_FOUND)
        list(APPEND WINDOW_FACTORY_REG RegisterX11WindowFactory)
        target_sources( ${COMPONENT} PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src/device/display_x11.cpp )
        target_link_libraries(${COMPONENT} PRIVATE ${X11_LIBRARIES} )
        target_include_directories(${COMPONENT} PRIVATE ${X11_INCLUDE_DIR} )
    endif()
    target_compile_definitions(${COMPONENT} PUBLIC "PANGO_DEFAULT_WIN_URI=\"x11\"")
endif()

# Create window_frameworks.h file for inclusion in library
# This loads windowing framework factories based on cmake configuration
CreateMethodCallFile(
    "${CMAKE_CURRENT_BINARY_DIR}/include/pangolin/display/window_frameworks.h"
    "pangolin" "LoadBuiltInWindowFrameworks" "${WINDOW_FACTORY_REG}"
)

target_link_libraries(${COMPONENT} PUBLIC pango_core pango_opengl pango_vars )

target_include_directories(${COMPONENT}
    PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include
    PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/include
)

# Embed font
include(EmbedBinaryFiles)
embed_binary_files( "${CMAKE_CURRENT_LIST_DIR}/src/fonts/*.ttf" "${CMAKE_CURRENT_BINARY_DIR}/fonts.cpp" )
target_sources( ${COMPONENT} PUBLIC "${CMAKE_CURRENT_BINARY_DIR}/fonts.cpp" )
