get_filename_component(COMPONENT ${CMAKE_CURRENT_LIST_DIR} NAME)

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    target_link_libraries(${COMPONENT} PRIVATE "-framework Cocoa" )
    target_sources( ${COMPONENT}
        PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/src/display_osx.mm
        ${CMAKE_CURRENT_LIST_DIR}/src/PangolinNSApplication.mm
        ${CMAKE_CURRENT_LIST_DIR}/src/PangolinNSGLView.mm
    )
    target_compile_definitions(${COMPONENT} PUBLIC "PANGO_DEFAULT_WIN_URI=\"cocoa\"")
    target_compile_options(${COMPONENT} PRIVATE
        $<$<OR:$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:GNU>>:
        -Wno-deprecated-declarations>
    )
    PangolinRegisterFactory(WindowInterface OsxWindow)
elseif(WIN32 OR WIN64)
    target_sources( ${COMPONENT} PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src/display_win.cpp )
    target_compile_definitions(${COMPONENT} PUBLIC "PANGO_DEFAULT_WIN_URI=\"winapi\"")
    PangolinRegisterFactory(WindowInterface WinWindow)
elseif(EMSCRIPTEN)
    target_sources( ${COMPONENT} PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src/display_emscripten.cpp )
    target_compile_definitions(${COMPONENT} PUBLIC "PANGO_DEFAULT_WIN_URI=\"emscripten\"")
    PangolinRegisterFactory(WindowInterface EmscriptenWindow)
else()
    find_package(X11 QUIET)
    find_package(OpenGL QUIET COMPONENTS EGL)
    if(X11_FOUND AND OpenGL_EGL_FOUND)
        target_sources( ${COMPONENT} PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src/display_x11.cpp )
        target_link_libraries(${COMPONENT} PRIVATE ${X11_LIBRARIES} )
        target_include_directories(${COMPONENT} PRIVATE ${X11_INCLUDE_DIR} )
        target_link_libraries(${COMPONENT} PRIVATE OpenGL::EGL)
        PangolinRegisterFactory(WindowInterface X11Window)
    endif()

    # Wayland
    find_package(PkgConfig)
    pkg_check_modules(WAYLAND_CLIENT wayland-client QUIET)
    pkg_check_modules(wayland-protocols QUIET wayland-protocols>=1.13)
    if(WAYLAND_CLIENT_FOUND AND wayland-protocols_FOUND)
        pkg_check_modules(WAYLAND_EGL wayland-egl REQUIRED)
        pkg_check_modules(WAYLAND_CURSOR wayland-cursor REQUIRED)
        pkg_check_modules(xkbcommon REQUIRED xkbcommon)

        target_sources( ${COMPONENT} PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src/display_wayland.cpp )

        target_link_libraries(${COMPONENT} PRIVATE ${X11_LIBRARIES}
            ${WAYLAND_CLIENT_LIBRARIES}
            ${WAYLAND_EGL_LIBRARIES}
            ${WAYLAND_CURSOR_LIBRARIES}
            ${xkbcommon_LIBRARIES}
        )

        # find Wayland protocols
        pkg_get_variable(WAYLAND_PROTOCOLS_DIR wayland-protocols pkgdatadir)

        # find 'wayland-scanner' executable
        pkg_get_variable(WAYLAND_SCANNER wayland-scanner wayland_scanner)

        # generate protocol implementation
        set(XDG_PROT_DEF "${WAYLAND_PROTOCOLS_DIR}/stable/xdg-shell/xdg-shell.xml")
        add_custom_command(
            OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/xdg-shell-client-protocol.h
            COMMAND ${WAYLAND_SCANNER} client-header ${XDG_PROT_DEF} xdg-shell-client-protocol.h)
        add_custom_command(
            OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/xdg-shell-protocol.c
            COMMAND ${WAYLAND_SCANNER} private-code ${XDG_PROT_DEF} xdg-shell-protocol.c
            DEPENDS xdg-shell-client-protocol.h)
        target_include_directories(${COMPONENT} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
        target_sources( ${COMPONENT} PRIVATE xdg-shell-protocol.c)

        # register window factory
        target_compile_definitions(${COMPONENT} PUBLIC "PANGO_DEFAULT_WIN_URI=\"wayland\"")
        PangolinRegisterFactory(WindowInterface WaylandWindow)
    else()
        target_compile_definitions(${COMPONENT} PUBLIC "PANGO_DEFAULT_WIN_URI=\"x11\"")
    endif()
endif()

# headless offscreen rendering via EGL
if(_LINUX_)
    set(OpenGL_GL_PREFERENCE "GLVND")
endif()
find_package(OpenGL QUIET COMPONENTS EGL)
if(OpenGL_EGL_FOUND)
    target_sources( ${COMPONENT} PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src/display_headless.cpp )
    target_link_libraries(${COMPONENT} PRIVATE ${OPENGL_egl_LIBRARY} )
    PangolinRegisterFactory(WindowInterface HeadlessWindow)
endif()

set_target_properties(
    ${COMPONENT} PROPERTIES VERSION ${PANGOLIN_VERSION} SOVERSION ${PANGOLIN_VERSION_MAJOR}
)

target_sources(${COMPONENT} PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src/window.cpp)
target_link_libraries(${COMPONENT} PUBLIC pango_core pango_opengl )
target_include_directories(${COMPONENT} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
install(DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/include" DESTINATION ${CMAKE_INSTALL_PREFIX})
install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/include" DESTINATION ${CMAKE_INSTALL_PREFIX})

# Create RegisterFactoriesWindowInterface() method call file to load built drivers and add as source
create_factory_registry_file( "${CMAKE_CURRENT_BINARY_DIR}/include/pangolin/factory" WindowInterface )
target_sources(${COMPONENT} PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/include/pangolin/factory/RegisterFactoriesWindowInterface.h")
