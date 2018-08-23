/* This file is part of the Pangolin Project.
 * http://github.com/stevenlovegrove/Pangolin
 *
 * Copyright (c) 2016 Steven Lovegrove
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

#pragma once
#include <exception>
#include <pangolin/platform.h>
#include <string>

namespace pangolin
{

class GlContextInterface
{
public:
    virtual ~GlContextInterface() {}
};

class WindowInterface
{
public:
    virtual ~WindowInterface() {}

    virtual void ToggleFullscreen() = 0;

    virtual void Move(int x, int y) = 0;

    virtual void Resize(unsigned int w, unsigned int h) = 0;

    /**
     * @brief MakeCurrent set the current context
     * to be called in a thread before accessing OpenGL
     */
    virtual void MakeCurrent() = 0;

    /**
     * @brief RemoveCurrent remove the current context
     * to be called at the end of a thread
     */
    virtual void RemoveCurrent() = 0;

    virtual void ProcessEvents() = 0;

    virtual void SwapBuffers() = 0;
};


struct PANGOLIN_EXPORT WindowException : std::exception
{
    WindowException(std::string str) : desc(str) {}
    WindowException(std::string str, std::string detail) {
        desc = str + "\n\t" + detail;
    }
    ~WindowException() throw() {}
    const char* what() const throw() { return desc.c_str(); }
    std::string desc;
};

struct PANGOLIN_EXPORT WindowExceptionNoKnownHandler : public WindowException
{
    WindowExceptionNoKnownHandler(const std::string& scheme)
        : WindowException("No known window handler for URI '" + scheme + "'")
    {
    }
};

}
