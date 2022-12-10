# Find Pangolin (https://github.com/stevenlovegrove/Pangolin)
find_package(Pangolin 0.8 REQUIRED)
include_directories(${Pangolin_INCLUDE_DIRS})

set(examples
    1.hello
    2.layouts
    3.widgets
    4.video
    5.video-seek
    6.overlay
    7.solids
    8.pixel-perfect
    9.flex
    10.shapes
    11.scene-group
    12.named_drawables
    13.plot-experiment
)

foreach(example IN LISTS examples)
    add_executable(${example} ${example}.cpp)
    target_link_libraries(${example} pango_context pango_video)
endforeach()
