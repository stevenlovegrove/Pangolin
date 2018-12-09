cmake_minimum_required(VERSION 3.7.0)
project(test_uuid CXX)

include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/catch)

file(GLOB headers ${CMAKE_SOURCE_DIR}/include/*.h)
file(GLOB SOURCES "test/*.cpp" "include/*.cpp")

if(BUILD_TESTS)
    include(CTest)
endif()

if(WIN32)
   message(status "Setting MSVC flags")
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHc /std:c++latest")
   add_definitions(-D_SCL_SECURE_NO_WARNINGS)
elseif(APPLE)
   message(status "Setting Clang flags")
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1z -fexceptions -g -Wall")
else()
   #include_directories(${LIBUUID_INCLUDE_DIR})
   message(status "Setting GCC flags")
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1z -fexceptions -g -Wall")
endif()

add_executable(test_uuid ${SOURCES} ${headers})

if(WIN32)
elseif(APPLE)
	find_library(CFLIB CoreFoundation)
	target_link_libraries(test_uuid ${CFLIB})
else()
    FIND_PATH(LIBUUID_INCLUDE_DIRS uuid/uuid.h)
    FIND_LIBRARY(LIBUUID_LIBRARIES uuid)

    IF (LIBUUID_LIBRARIES AND LIBUUID_INCLUDE_DIRS)       
        SET(LIBUUID_FOUND 1)
        IF (NOT LibUuid_FIND_QUIETLY)
            MESSAGE(STATUS "Found libuuid: ${LIBUUID_LIBRARIES}")
        ENDIF ( NOT LibUuid_FIND_QUIETLY )
    ELSE (LIBUUID_LIBRARIES AND LIBUUID_INCLUDE_DIRS)     
        IF (LibUuid_FIND_REQUIRED)
            MESSAGE(SEND_ERROR "Could NOT find libuuid")
        ELSE (LibUuid_FIND_REQUIRED)
            IF (NOT LIBUUID_FIND_QUIETLY)
                MESSAGE(STATUS "Could NOT find libuuid")     
            ENDIF (NOT LIBUUID_FIND_QUIETLY)
        ENDIF (LibUuid_FIND_REQUIRED)
    ENDIF (LIBUUID_LIBRARIES AND LIBUUID_INCLUDE_DIRS)

    MARK_AS_ADVANCED(LIBUUID_LIBRARIES LIBUUID_INCLUDE_DIRS)
    
    include_directories(${LIBUUID_INCLUDE_DIRS})

	#find_package(Libuuid REQUIRED NO_MODULE)
	#if (NOT LIBUUID_FOUND)
	#	message(FATAL_ERROR
    #   "You might need to run 'sudo apt-get install uuid-dev' or similar")
	#endif()
	target_link_libraries(test_uuid ${LIBUUID_LIBRARY})
endif()

message(status "** CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")

if(BUILD_TESTS)
    enable_testing()

    add_test(NAME "test_uuid" COMMAND "test_uuid" "-r compact")
    set_tests_properties("test_uuid"
        PROPERTIES
        PASS_REGULAR_EXPRESSION "Passed all.*")
    set_tests_properties("test_uuid"
        PROPERTIES
        FAIL_REGULAR_EXPRESSION "Failed \\d+ test cases")
    set_tests_properties("test_uuid"
        PROPERTIES
        TIMEOUT 120)
endif()