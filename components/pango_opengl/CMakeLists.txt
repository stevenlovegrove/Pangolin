get_filename_component(COMPONENT ${CMAKE_CURRENT_LIST_DIR} NAME)

include(EmbedBinaryFiles)
include(EmbedShaderFiles)
file(GLOB BINARY_FILES_TO_EMBED "${CMAKE_CURRENT_LIST_DIR}/src/fonts/*.ttf")
file(GLOB_RECURSE SHADER_FILES_TO_EMBED "${CMAKE_CURRENT_LIST_DIR}/shaders/*.glsl*")
embed_binary_files_rule(fonts.cpp ${BINARY_FILES_TO_EMBED})
embed_shader_files_rule(shaders.cpp "${SHADER_FILES_TO_EMBED}")

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
    ${CMAKE_CURRENT_BINARY_DIR}/fonts.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/shaders.cpp
)

find_package (Eigen3 REQUIRED QUIET)
message(STATUS "Found Eigen: '${EIGEN3_INCLUDE_DIRS}'")
target_compile_definitions(${COMPONENT} PUBLIC HAVE_EIGEN HAVE_GLEW)

target_link_libraries(${COMPONENT} PUBLIC pango_core pango_image Eigen3::Eigen)
target_include_directories(${COMPONENT} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
install(DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/include"
  DESTINATION ${CMAKE_INSTALL_PREFIX}
)

if(EMSCRIPTEN)
    target_compile_definitions(${COMPONENT} PUBLIC HAVE_GLES HAVE_GLES_2)
    target_sources( ${COMPONENT} PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src/compat/gl2engine.cpp)
else()
    if(_LINUX_)
        set(OpenGL_GL_PREFERENCE "GLVND")
    endif()
    find_package(OpenGL REQUIRED QUIET)
    find_package(GLEW REQUIRED QUIET)
    target_include_directories( ${COMPONENT} PUBLIC
        $<BUILD_INTERFACE:${OPENGL_INCLUDE_DIR}>
        $<BUILD_INTERFACE:${GLEW_INCLUDE_DIR}>
    )
    target_link_libraries( ${COMPONENT} PUBLIC
        ${GLEW_LIBRARY} ${OPENGL_LIBRARIES}
    )
endif()
