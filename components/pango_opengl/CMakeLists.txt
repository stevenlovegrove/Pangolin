get_filename_component(COMPONENT ${CMAKE_CURRENT_LIST_DIR} NAME)

target_sources( ${COMPONENT}
PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/src/glchar.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/gldraw.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/glfont.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/gltext.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/glpangoglu.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/gltexturecache.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/viewport.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/opengl_render_state.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/stb_truetype.h
)

target_link_libraries(${COMPONENT} PUBLIC pango_core pango_image)
target_include_directories(${COMPONENT} PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include )

find_package(OpenGL REQUIRED QUIET)
find_package(GLEW REQUIRED QUIET)
target_compile_definitions(${COMPONENT} PUBLIC HAVE_GLEW)
target_include_directories(${COMPONENT} PUBLIC ${GLEW_INCLUDE_DIR} ${OPENGL_INCLUDE_DIR})
target_link_libraries(${COMPONENT} PUBLIC ${GLEW_LIBRARY} ${OPENGL_LIBRARIES})

if(WIN32 OR WIN64)
    target_compile_definitions(${COMPONENT} PUBLIC _WIN_)
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    target_compile_definitions(${COMPONENT} PUBLIC _OSX_)
endif()
