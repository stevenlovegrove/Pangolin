/* This file is part of the Pangolin Project.
 * http://github.com/stevenlovegrove/Pangolin
 *
 * Copyright (c) 2011 Steven Lovegrove, Richard Newcombe
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef PANGOLIN_GLINCLUDE_H
#define PANGOLIN_GLINCLUDE_H

//////////////////////////////////////////////////////////
// Attempt to portably include Necessary OpenGL headers
//////////////////////////////////////////////////////////

#include "platform.h"

#ifdef _WIN_
#include <Windows.h>
#endif

#ifndef _ANDROID_
#include <GL/glew.h>
#endif

#ifdef HAVE_GLUT
    #ifdef HAVE_APPLE_OPENGL_FRAMEWORK
        #include <GLUT/glut.h>
        #define HAVE_GLUT_APPLE_FRAMEWORK
    
        inline void glutBitmapString(void* font, const unsigned char* str)
        {
            const unsigned char* s = str;
            while(*s != 0) {
                glutBitmapCharacter(font, *s);
                ++s;
            }
        }
    #else
        #include <GL/freeglut.h>
    #endif // HAVE_APPLE_OPENGL_FRAMEWORK
#endif // HAVE_GLUT

#ifdef _OSX_
    #include <OpenGL/gl.h>
#elif defined(_ANDROID_)
    #include <EGL/egl.h>
    #include <GLES/gl.h>
//    #define GLdouble GLfloat
    #define GLdouble GLfloat
#else
    #include <GL/gl.h>
#endif

#endif // PANGOLIN_GLINCLUDE_H
