add_subdirectory(VideoViewer)
add_subdirectory(VideoConvert)
add_subdirectory(VideoJson)
add_subdirectory(Plotter)

if(NOT EMSCRIPTEN)
    add_subdirectory(ModelViewer)
endif()
