add_library(mio_full_winapi INTERFACE)
target_link_libraries(mio_full_winapi
    INTERFACE mio_base
)
add_library(mio::mio_full_winapi ALIAS mio_full_winapi)

add_library(mio INTERFACE)
target_link_libraries(mio
    INTERFACE mio_full_winapi
)
target_compile_definitions(mio
    INTERFACE WIN32_LEAN_AND_MEAN NOMINMAX
)
install(TARGETS mio_full_winapi EXPORT mioConfig)
