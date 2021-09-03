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
    if(X11_FOUND)
        target_sources( ${COMPONENT} PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src/display_x11.cpp )
        target_link_libraries(${COMPONENT} PRIVATE ${X11_LIBRARIES} )
        target_include_directories(${COMPONENT} PRIVATE ${X11_INCLUDE_DIR} )
    endif()
    target_compile_definitions(${COMPONENT} PUBLIC "PANGO_DEFAULT_WIN_URI=\"x11\"")
    PangolinRegisterFactory(WindowInterface X11Window)
endif()

# headless offscreen rendering via EGL
if(_LINUX_)
    set(OpenGL_GL_PREFERENCE "GLVND")
endif()
find_package(OpenGL QUIET COMPONENTS EGL)
if(OpenGL_EGL_FOUND)
    target_sources( ${COMPONENT} PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src/display_headless.cpp )
    target_link_libraries(${COMPONENT} PRIVATE ${OPENGL_egl_LIBRARY} )
endif()


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
