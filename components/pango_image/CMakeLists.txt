get_filename_component(COMPONENT ${CMAKE_CURRENT_LIST_DIR} NAME)

option(BUILD_PANGOLIN_LIBPNG "Build support for libpng image input" ON)
if(BUILD_PANGOLIN_LIBPNG)
    find_package(PNG QUIET)
    if(PNG_FOUND)
        target_compile_definitions(${COMPONENT} PRIVATE HAVE_PNG)
        target_include_directories(${COMPONENT} PRIVATE ${PNG_INCLUDE_DIR} )
        target_link_libraries(${COMPONENT} PRIVATE ${PNG_LIBRARY} ${ZLIB_LIBRARY})
        message(STATUS "libpng Found and Enabled")
    endif()
endif()

option(BUILD_PANGOLIN_LIBJPEG "Build support for libjpeg image input" ON)
if(BUILD_PANGOLIN_LIBJPEG)
    find_package(JPEG QUIET)
    if(JPEG_FOUND)
        target_compile_definitions(${COMPONENT} PRIVATE HAVE_JPEG)
        target_include_directories(${COMPONENT} PRIVATE ${JPEG_INCLUDE_DIR} )
        target_link_libraries(${COMPONENT} PRIVATE ${JPEG_LIBRARY})
        message(STATUS "libjpeg Found and Enabled")
    endif()
endif()

option(BUILD_PANGOLIN_LIBTIFF "Build support for libtiff image input" ON)
if(BUILD_PANGOLIN_LIBTIFF)
    find_package(TIFF QUIET)
    if(TIFF_FOUND)
        target_compile_definitions(${COMPONENT} PRIVATE HAVE_TIFF)
        target_include_directories(${COMPONENT} PRIVATE ${TIFF_INCLUDE_DIR} )
        target_link_libraries(${COMPONENT} PRIVATE ${TIFF_LIBRARY})
        message(STATUS "libtiff Found and Enabled")
    endif()
endif()

option(BUILD_PANGOLIN_LIBOPENEXR "Build support for libopenexr image input" ON)
if(BUILD_PANGOLIN_LIBOPENEXR)
    find_package(OpenEXR QUIET)
    if(OpenEXR_FOUND)
        target_compile_definitions(${COMPONENT} PRIVATE HAVE_OPENEXR)
        target_include_directories(${COMPONENT} PRIVATE ${OpenEXR_INCLUDE_DIR} )
        target_link_libraries(${COMPONENT} PRIVATE ${OpenEXR_LIBRARY})
        target_compile_options(${COMPONENT} PRIVATE
            $<$<OR:$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:GNU>>:
            -Wno-deprecated-register -Wno-deprecated>
        )
        message(STATUS "libopenexr Found and Enabled")
    endif()
endif()

option(BUILD_PANGOLIN_LZ4 "Build support for liblz4 compression" ON)
if(BUILD_PANGOLIN_LZ4)
    find_package(Lz4 QUIET)
    if(Lz4_FOUND)
        target_compile_definitions(${COMPONENT} PRIVATE HAVE_LZ4)
        target_include_directories(${COMPONENT} PRIVATE ${Lz4_INCLUDE_DIRS} )
        target_link_libraries(${COMPONENT} PRIVATE ${Lz4_LIBRARIES})
        message(STATUS "liblz4 Found and Enabled")
    endif()
endif()

option(BUILD_PANGOLIN_ZSTD "Build support for libzstd compression" ON)
if(BUILD_PANGOLIN_ZSTD)
    find_package(zstd QUIET)
    if(zstd_FOUND)
        target_compile_definitions(${COMPONENT} PRIVATE HAVE_ZSTD)
        target_include_directories(${COMPONENT} PRIVATE ${zstd_INCLUDE_DIR} )
        target_link_libraries(${COMPONENT} PRIVATE ${zstd_LIBRARY})
        message(STATUS "libzstd Found and Enabled")
    endif()
endif()

target_sources( ${COMPONENT}
PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/src/pixel_format.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/image_io.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/image_io_exr.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/image_io_jpg.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/image_io_lz4.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/image_io_packed12bit.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/image_io_pango.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/image_io_png.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/image_io_ppm.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/image_io_raw.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/image_io_tga.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/image_io_bmp.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/image_io_zstd.cpp
)

target_link_libraries(${COMPONENT} PUBLIC pango_core)
target_include_directories(${COMPONENT} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
install(DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/include"
  DESTINATION ${CMAKE_INSTALL_PREFIX}
)
