add_subdirectory(BasicOpenGL)

add_subdirectory(HelloPangolin)
add_subdirectory(HelloPangolinThreads)
add_subdirectory(SimpleDisplay)
add_subdirectory(SimpleMultiDisplay)
add_subdirectory(SimpleDisplayImage)
add_subdirectory(SimplePlot)
add_subdirectory(SimpleVideo)
add_subdirectory(SimpleRecord)

if(NOT EMSCRIPTEN)
    add_subdirectory(HelloPangolinOffscreen)
    add_subdirectory(SimpleScene) # undefined symbol: glInitNames
endif()
