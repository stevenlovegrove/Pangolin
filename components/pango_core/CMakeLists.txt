get_filename_component(COMPONENT ${CMAKE_CURRENT_LIST_DIR} NAME)

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    target_compile_definitions(${COMPONENT} PUBLIC "_OSX_")
elseif(WIN32 OR WIN64)
    target_compile_definitions(${COMPONENT} PUBLIC "_WIN_")
elseif(EMSCRIPTEN)
    target_compile_definitions(${COMPONENT} PUBLIC "_EMSCRIPTEN_")
else()
    target_compile_definitions(${COMPONENT} PUBLIC "_LINUX_")
endif()

target_sources( ${COMPONENT}
PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/src/file_extension.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/file_utils.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/sigstate.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/threadedfilebuf.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/avx_math.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/uri.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/param_set.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/factory/factory_registry.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/factory/factory_help.cpp
)

if (UNIX)
    target_sources( ${COMPONENT} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/src/posix/condition_variable.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/posix/semaphore.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/posix/shared_memory_buffer.cpp
    )
    if (NOT APPLE)
        target_link_libraries(${COMPONENT} PUBLIC rt)
    endif()
elseif(WIN32 OR WIN64)
    target_link_libraries(${COMPONENT} PRIVATE shlwapi.lib)
endif()

set_target_properties(
    ${COMPONENT} PROPERTIES VERSION ${PANGOLIN_VERSION} SOVERSION ${PANGOLIN_VERSION_MAJOR}
)

target_compile_features(${COMPONENT} PUBLIC cxx_decltype_auto )
target_include_directories(${COMPONENT} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
install(DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/include"
  DESTINATION ${CMAKE_INSTALL_PREFIX}
)

find_package(Threads QUIET)
if(Threads_FOUND)
    target_link_libraries(${COMPONENT} PUBLIC Threads::Threads)
endif()

if(BUILD_TESTS)
    add_executable(test_uris ${CMAKE_CURRENT_LIST_DIR}/tests/tests_uri.cpp)
    target_link_libraries(test_uris PRIVATE Catch2::Catch2WithMain ${COMPONENT})
    catch_discover_tests(test_uris)
endif()
