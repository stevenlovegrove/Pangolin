# Configure build environment to automatically generate APK's instead of executables.
if(ANDROID)
    # virtual targets which we'll add apks and push actions to.
    add_custom_target( apk )
    add_custom_target( push )
    add_custom_target( run )

    # Reset output directories to be in binary folder (rather than source)
    set(LIBRARY_OUTPUT_PATH_ROOT ${CMAKE_CURRENT_BINARY_DIR})
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${LIBRARY_OUTPUT_PATH_ROOT}/libs/${ANDROID_NDK_ABI_NAME})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${LIBRARY_OUTPUT_PATH_ROOT}/libs/${ANDROID_NDK_ABI_NAME})
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${LIBRARY_OUTPUT_PATH_ROOT}/bin/${ANDROID_NDK_ABI_NAME})
    
    macro( create_android_manifest_xml filename prog_name package_name activity_name)
        file( WRITE ${filename}
"<?xml version=\"1.0\" encoding=\"utf-8\"?>
<!-- BEGIN_INCLUDE(manifest) -->
<manifest xmlns:android=\"http://schemas.android.com/apk/res/android\"
        package=\"${package_name}\"
        android:versionCode=\"1\"
        android:versionName=\"1.0\">

    <!-- This is the platform API where NativeActivity was introduced. -->
    <uses-sdk android:minSdkVersion=\"14\" />
    <uses-feature android:name=\"android.hardware.camera\" />
    <uses-permission android:name=\"android.permission.CAMERA\"/>
    <uses-permission android:name=\"android.permission.WRITE_EXTERNAL_STORAGE\"/>
    <uses-permission android:name=\"android.permission.READ_EXTERNAL_STORAGE\"/>

    <!-- This .apk has no Java code itself, so set hasCode to false. -->
    <application android:label=\"${activity_name}\" android:hasCode=\"false\">

        <!-- Our activity is the built-in NativeActivity framework class.
             This will take care of integrating with our NDK code. -->
        <activity android:name=\"android.app.NativeActivity\"
                android:label=\"${activity_name}\"
                android:screenOrientation=\"landscape\"
                android:configChanges=\"orientation|keyboardHidden\"
                android:theme=\"@android:style/Theme.NoTitleBar.Fullscreen\"
                >
            <!-- Tell NativeActivity the name of our .so -->
            <meta-data android:name=\"android.app.lib_name\"
                    android:value=\"${prog_name}_start\" />
            <intent-filter>
                <action android:name=\"android.intent.action.MAIN\" />
                <category android:name=\"android.intent.category.LAUNCHER\" />
            </intent-filter>
        </activity>
    </application>

</manifest> 
<!-- END_INCLUDE(manifest) -->" )        
    endmacro()

    macro( create_bootstrap_library prog_name package_name)
        set(bootstrap_cpp "${CMAKE_CURRENT_BINARY_DIR}/${prog_name}_start.cpp" )
        file( WRITE ${bootstrap_cpp}
"#include <android/native_activity.h>
#include <android/log.h>
#include <dlfcn.h>
#include <errno.h>
#include <stdlib.h>
#include <cstdio>

#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, \"AndroidUtils.cmake\", __VA_ARGS__))
#define LIB_PATH \"/data/data/${package_name}/lib/\"

void * load_lib(const char * l) {
    void * handle = dlopen(l, RTLD_NOW | RTLD_GLOBAL);
    if (!handle) {
        LOGE( \"dlopen('%s'): %s\", l, strerror(errno) );
    }
    return handle;
}

void ANativeActivity_onCreate(ANativeActivity * app, void * ud, size_t udsize) {
    #include \"${prog_name}_shared_load.h\"
    void (*main)(ANativeActivity*, void*, size_t);
    *(void **) (&main) = dlsym(load_lib( LIB_PATH \"lib${prog_name}.so\"), \"ANativeActivity_onCreate\");
    if (!main) {
        LOGE( \"undefined symbol ANativeActivity_onCreate\" );
        exit(1);
    }
    (*main)(app, ud, udsize);
}" )
        add_library( "${prog_name}_start" SHARED ${bootstrap_cpp} )
        target_link_libraries( "${prog_name}_start" android log )
        add_dependencies( ${prog_name} "${prog_name}_start" )
    endmacro()

    macro( android_update android_project_name)
        # Generate ant build scripts for making APK
        execute_process(
            COMMAND android update project --name ${android_project_name} --path . --target android-17 --subprojects
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        )
    endmacro()

    # Override add_executable to build android .so instead!
    macro( add_executable prog_name)
        set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/libs/${ANDROID_NDK_ABI_NAME})
        add_library( ${prog_name} SHARED ${ARGN} )

        # Add required link libs for android
        target_link_libraries(${prog_name} log android )

        # Create manifest required for APK
        set(ANDROID_PACKAGE_NAME "com.github.stevenlovegrove.pangolin.${prog_name}")
        create_android_manifest_xml(
            "${CMAKE_CURRENT_BINARY_DIR}/AndroidManifest.xml" "${prog_name}"
            "${ANDROID_PACKAGE_NAME}" "${prog_name}"
        )

        # Create library that will launch this program and load shared libs
        create_bootstrap_library( ${prog_name} ${ANDROID_PACKAGE_NAME} )

        # Generate ant build system for APK
        android_update( ${prog_name} )

        # Target to invoke ant build system for APK
        set( APK_FILE "${CMAKE_CURRENT_BINARY_DIR}/bin/${prog_name}-debug.apk" )
        add_custom_command(
            OUTPUT ${APK_FILE}
            COMMAND ant debug
            DEPENDS ${prog_name}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        )

        # Target to install on device
        add_custom_target( ${prog_name}-apk
            DEPENDS ${APK_FILE}
        )
        add_dependencies(apk ${prog_name}-apk)

        # Target to install on device
        add_custom_target( ${prog_name}-push
            COMMAND adb install -r ${APK_FILE}
            DEPENDS ${APK_FILE}
        )
        add_dependencies(push ${prog_name}-push)

        # install and run on device
        add_custom_target( ${prog_name}-run
            COMMAND adb shell am start -n ${ANDROID_PACKAGE_NAME}/android.app.NativeActivity
            DEPENDS ${prog_name}-push
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        )
        add_dependencies(run ${prog_name}-run)

        # Clear shared library loading header
        file( WRITE "${CMAKE_CURRENT_BINARY_DIR}/${prog_name}_shared_load.h" "")
    endmacro()

    macro( add_to_depend_libs prog_name depend_file lib_name )
            # dependents of lib
            get_target_property(TARGET_LIBS ${lib_name} IMPORTED_LINK_INTERFACE_LIBRARIES_NOCONFIG)
            foreach(SUBLIB ${TARGET_LIBS})
                if(SUBLIB)
                    add_to_depend_libs( ${prog_name} ${depend_file} ${SUBLIB} )
                endif()
            endforeach()

            # lib itself
            get_target_property(TARGET_LIB ${lib_name} IMPORTED_LOCATION_NOCONFIG)
            if(TARGET_LIB)
                get_filename_component(target_filename ${TARGET_LIB} NAME)
                file( APPEND ${depend_file} "load_lib(LIB_PATH \"${target_filename}\" );\n")
                add_custom_command(TARGET ${prog_name} POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    ${TARGET_LIB} "${CMAKE_CURRENT_BINARY_DIR}/libs/${ANDROID_NDK_ABI_NAME}"
                )
            endif()
    endmacro()


    macro( target_link_libraries prog_name)
        # _target_link_libraries corresponds to original
        _target_link_libraries( ${prog_name} ${ARGN} )

        # Add to shared library loading header.
        set(depend_file "${CMAKE_CURRENT_BINARY_DIR}/${prog_name}_shared_load.h" )
        foreach( LIB ${ARGN} )
            add_to_depend_libs( ${prog_name} ${depend_file} ${LIB} )
        endforeach()
    endmacro()

endif()
