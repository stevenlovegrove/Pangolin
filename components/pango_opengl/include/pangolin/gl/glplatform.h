#pragma once

//////////////////////////////////////////////////////////
// Attempt to portably include Necessary OpenGL headers
//////////////////////////////////////////////////////////

#include <pangolin/platform.h>
#include <pangolin/utils/logging.h>

#ifdef _WIN_
    // Define maths quantities when using <cmath> to match posix systems
    #ifndef _USE_MATH_DEFINES
    #  define _USE_MATH_DEFINES
    #endif

    // Don't define min / max macros in windows.h or other unnecessary macros
    #ifndef NOMINMAX
    #  define NOMINMAX
    #endif
    #ifndef WIN32_LEAN_AND_MEAN
    #  define WIN32_LEAN_AND_MEAN
    #endif
    #include <Windows.h>

    // Undef nuisance Windows.h macros which interfere with our methods
    #undef LoadImage
    #undef near
    #undef far
    #undef ERROR
#endif

#ifdef HAVE_GLEW
    #include <GL/glew.h>
#endif

#ifdef HAVE_GLES
    #if defined(_ANDROID_)
        #include <EGL/egl.h>
        #ifdef HAVE_GLES_2
            #include <GLES2/gl2.h>
            #include <GLES2/gl2ext.h>
        #else
            #include <GLES/gl.h>
            #define GL_GLEXT_PROTOTYPES
            #include <GLES/glext.h>
        #endif
    #elif defined(_APPLE_IOS_)
        #include <OpenGLES/ES2/gl.h>
        #include <OpenGLES/ES2/glext.h>
    #endif
#else
    #ifdef _OSX_
        #define GL_SILENCE_DEPRECATION
        #include <OpenGL/gl.h>
    #else
        #include <GL/gl.h>
    #endif
#endif // HAVE_GLES

// For glErrorString
#include <pangolin/gl/glpangoglu.h>

#define PANGO_GL(FUNC) \
  [&](){ \
    FUNC; \
    GLenum gl_error = glGetError(); \
    if( gl_error != GL_NO_ERROR) { \
        pangolin::Log::instance().log(pangolin::Log::Kind::Error, __FILE__, PANGO_FUNCTION, __LINE__, \
        "", "GL Error ({}): {}", gl_error, pangolin::glErrorString(gl_error) ); \
        return false; \
    } \
    return true; \
  }()

#define PANGO_GL_CHECK() \
    PANGO_GL({})
