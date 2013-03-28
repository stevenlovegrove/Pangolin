#ifndef PANGOLIN_CONFIG_H
#define PANGOLIN_CONFIG_H

/*
 * Configuration Header for Pangolin
 */

/// Version
#define PANGOLIN_VERSION_MAJOR 0
#define PANGOLIN_VERSION_MINOR 1

/// Pangolin options
#define BUILD_PANGOLIN_VARS
#define BUILD_PANGOLIN_VIDEO

/// Configured libraries
#define HAVE_CUDA
#define HAVE_CVARS
#define HAVE_EIGEN
#define HAVE_TOON
#define HAVE_DC1394
/* #undef HAVE_V4L */
#define HAVE_FFMPEG
#define HAVE_OPENNI

#define HAVE_GLUT
/* #undef HAVE_FREEGLUT */
#define HAVE_APPLE_OPENGL_FRAMEWORK
#define HAVE_MODIFIED_OSXGLUT

#define HAVE_BOOST_GIL
#define HAVE_PNG
#define HAVE_JPEG
#define HAVE_TIFF

/// Platform
#define _UNIX_
/* #undef _WIN_ */
#define _OSX_
/* #undef _LINUX_ */

/// Compiler
#define _GCC_
/* #undef _MSVC_ */

#endif //PANGOLIN_CONFIG_H
