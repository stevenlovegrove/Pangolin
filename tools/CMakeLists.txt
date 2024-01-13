add_subdirectory(VideoConvert)
add_subdirectory(VideoJson)
add_subdirectory(Plotter)

if(NOT EMSCRIPTEN)
    add_subdirectory(VideoViewer) # undefined symbol: glRotatef
    add_subdirectory(ModelViewer) # undefined symbol: glColorPointer
endif()
