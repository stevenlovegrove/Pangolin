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

#ifndef PANGOLIN_TEXTUREVIEW_H
#define PANGOLIN_TEXTUREVIEW_H

#include <pangolin/platform.h>
#include <pangolin/display.h>
#include <pangolin/gl.h>

namespace pangolin
{

template<typename T>
class GlTextureViewTemplatedType : public T, public View, public Handler
{
public:
    GlTextureViewTemplatedType(GLint width, GLint height, GLint internal_format = GL_RGBA8, bool flipy = false )
        : T(width, height, internal_format), flipy(flipy)
    {
        this->SetAspect((double)width/(double)height);
        this->SetHandler(this);
        selected[0] = 0;
        selected[1] = 0;
    }

    virtual void Keyboard(View&, unsigned char key, int x, int y, bool pressed)
    {
    }

    virtual void Mouse(View&, MouseButton button, int x, int y, bool pressed, int button_state)
    {
        WindowToImage(x,y, selected[0], selected[1]);

    }

    virtual void MouseMotion(View&, int x, int y, int button_state)
    {
        WindowToImage(x,y, selected[0], selected[1]);
    }

    virtual void Render()
    {
        this->Activate();

        if(flipy) {
            this->RenderToViewportFlipY();
        }else{
            this->RenderToViewport();
        }
    }

#ifdef HAVE_EIGEN
    Eigen::Vector2d GetSelectedPoint() const
    {
        return Eigen::Vector2d(selected[0], selected[1]);
    }
#endif

protected:
    void WindowToImage(int wx, int wy, float& ix, float& iy )
    {
        const int vx = wx - v.l;
        const int vy = wy - v.b;
        ix = (float)(GlTexture::width)  * (float)vx /(float)v.w;
        iy = (float)(GlTexture::height) * (float)vy /(float)v.h;
        ix = std::max(0.0f,std::min(ix, (float)GlTexture::width));
        iy = std::max(0.0f,std::min(iy, (float)GlTexture::height));
        if(flipy) iy = GlTexture::height - iy;
    }

    bool flipy;
    float selected[2];
};

typedef GlTextureViewTemplatedType<GlTexture> GlTextureView;

}

#endif // PANGOLIN_TEXTUREVIEW_H
