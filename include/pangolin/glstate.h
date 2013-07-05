/* This file is part of the Pangolin Project.
 * http://github.com/stevenlovegrove/Pangolin
 *
 * Copyright (c) 2013 Vincent Mamo, Steven Lovegrove
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

#ifndef PANGOLIN_GLSTATE_H
#define PANGOLIN_GLSTATE_H

#include <pangolin/glinclude.h>
#include <stack>

namespace pangolin
{

class GlState {

    class CapabilityEnabled {
    public:

        CapabilityEnabled(GLenum cap, GLboolean enable)
            : m_cap(cap), m_enable(enable)
        {

        }

        void Apply() {
            if(m_enable) {
                ::glEnable(m_cap);
            }else{
                ::glDisable(m_cap);
            }
        }

        void UnApply() {
            if(m_enable) {
                ::glDisable(m_cap);
            }else{
                ::glEnable(m_cap);
            }
        }

    protected:
        GLenum m_cap;
        GLboolean m_enable;
    };

public:
    GlState()
    {
        m_DepthMaskCalled = false;
        m_ShadeModelCalled = false;
        m_ColorMaskCalled = false;
        m_ViewportCalled = false;
    }

    ~GlState() {
        //  Restore original state
        while (!m_history.empty()) {
            m_history.top().UnApply();
            m_history.pop();
        }

        if (m_DepthMaskCalled) {
            ::glDepthMask(m_OriginalDepthMask);
        }

        if (m_ShadeModelCalled) {
            ::glShadeModel(m_OriginalShadeModel);
        }

        if (m_ColorMaskCalled) {
            ::glColorMask(m_OriginalColorMask[0], m_OriginalColorMask[1], m_OriginalColorMask[2], m_OriginalColorMask[3]);
        }

        if (m_ViewportCalled) {
            ::glViewport(m_OriginalViewport[0], m_OriginalViewport[1], m_OriginalViewport[2], m_OriginalViewport[3]);
        }
    }

    static inline GLboolean EnableValue(GLenum cap)
    {
        GLboolean curVal;
        glGetBooleanv(cap, &curVal);
        return curVal;
    }

    inline void glEnable(GLenum cap)
    {
        if(!EnableValue(cap)) {
            m_history.push(CapabilityEnabled(cap,false));
            ::glEnable(cap);
        }
    }


    inline void glDisable(GLenum cap)
    {
        if(EnableValue(cap)) {
            m_history.push(CapabilityEnabled(cap,true));
           ::glDisable(cap);
        }
    }

    bool m_DepthMaskCalled;
    GLboolean m_OriginalDepthMask;
    inline void glDepthMask(GLboolean flag)
    {
        m_DepthMaskCalled = true;
        glGetBooleanv(GL_DEPTH_WRITEMASK, &m_OriginalDepthMask);
        ::glDepthMask(flag);
    }

    bool m_ShadeModelCalled;
    GLint m_OriginalShadeModel;
    inline void glShadeModel(GLint mode)
    {
        m_ShadeModelCalled = true;
        glGetIntegerv(GL_SHADE_MODEL, &m_OriginalShadeModel);
        ::glShadeModel(mode);
    }

    bool m_ColorMaskCalled;
    GLboolean m_OriginalColorMask[4];
    inline void glColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha)
    {
        m_ColorMaskCalled = true;
        glGetBooleanv(GL_COLOR_WRITEMASK,  m_OriginalColorMask);
        ::glColorMask(red, green, blue, alpha);
    }

    bool m_ViewportCalled;
    GLint m_OriginalViewport[4];
    inline void glViewport(GLint x, GLint y, GLsizei width, GLsizei height)
    {
        m_ViewportCalled = true;
        glGetIntegerv(GL_VIEWPORT, m_OriginalViewport);
        ::glViewport(x, y, width, height);
    }

    std::stack<CapabilityEnabled> m_history;
};

}

#endif // PANGOLIN_GLSTATE_H
