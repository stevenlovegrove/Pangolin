## Compiler configuration
IF(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_GNUCC)
  SET(_GCC_ 1)
ENDIF()

IF(MSVC)
    SET(_MSVC_ 1)
ENDIF()

## Platform configuration

IF(WIN32 OR WIN64)
    SET(_WIN_ 1)
ENDIF()

IF(UNIX)
    SET(_UNIX_ 1)
ENDIF()

IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    SET(_OSX_ 1)
ENDIF()

IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    SET(_LINUX_ 1)
ENDIF()

IF(ANDROID)
    SET(_ANDROID_ 1)
ENDIF()

IF(IOS)
    SET(_APPLE_IOS_ 1)
ENDIF()



## Default search paths

IF(_WIN_)
    LIST(APPEND CMAKE_INCLUDE_PATH "c:/dev/sysroot/usr/include")
    LIST(APPEND CMAKE_LIBRARY_PATH "c:/dev/sysroot/usr/lib")
    LIST(APPEND CMAKE_LIBRARY_PATH "c:/dev/sysroot/usr/bin")
ENDIF()
