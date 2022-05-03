# Find Pangolin (https://github.com/stevenlovegrove/Pangolin)
find_package(Pangolin 0.8 REQUIRED)
include_directories(${Pangolin_INCLUDE_DIRS})

set(sample_srcs
    1_gl_intro_classic_triangle.cpp
    1_gl_intro_pango_triangle.cpp
    2_gl_intro_classic_triangle_vbo.cpp
    2_gl_intro_pango_triangle_vbo.cpp
    3_gl_intro_classic_triangle_vbo_shader.cpp
    3_gl_intro_pango_triangle_vbo_shader.cpp
    4_gl_intro_viewport.cpp
    5_gl_intro_view_transforms.cpp
)

foreach( sample_src ${sample_srcs})
    get_filename_component(sample_name ${sample_src} NAME_WE)
    set(target_name "tutorial_${sample_name}")
    add_executable(${target_name} ${sample_src})
    target_link_libraries(${target_name} pango_display)
endforeach()

