# Find Pangolin (https://github.com/stevenlovegrove/Pangolin)
find_package(Pangolin 0.4 REQUIRED)
include_directories(${Pangolin_INCLUDE_DIRS})

file(GLOB sample_srcs "*.cpp")
foreach( sample_src ${sample_srcs})
    get_filename_component(sample_name ${sample_src} NAME_WE)
    set(target_name "tutorial_${sample_name}")
    add_executable(${target_name} ${sample_src})
    target_link_libraries(${target_name} pango_display)
endforeach()

