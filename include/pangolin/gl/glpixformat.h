/* This file is part of the Pangolin Project.
 * http://github.com/stevenlovegrove/Pangolin
 *
 * Copyright (c) 2014 Steven Lovegrove
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

#ifndef PANGOLIN_GLPANGOPIXFORMAT_H
#define PANGOLIN_GLPANGOPIXFORMAT_H

#include <pangolin/gl/glplatform.h>
#include <pangolin/gl/gltexturecache.h>
#include <pangolin/image/image_common.h>

namespace pangolin {

// This class may dissapear in the future
struct GlPixFormat
{
    GlPixFormat() {}

    GlPixFormat(const pangolin::VideoPixelFormat& fmt)
    {
        switch( fmt.channels) {
        case 1: glformat = GL_LUMINANCE; break;
        case 3: glformat = (fmt.format == "BGR24") ? GL_BGR : GL_RGB; break;
        case 4: glformat = (fmt.format == "BGRA24") ? GL_BGRA : GL_RGBA; break;
        default: throw std::runtime_error("Unable to display video format");
        }

        switch (fmt.channel_bits[0]) {
        case 8: gltype = GL_UNSIGNED_BYTE; break;
        case 16: gltype = GL_UNSIGNED_SHORT; break;
        case 32: gltype = GL_FLOAT; break;
        default: throw std::runtime_error("Unknown channel format");
        }
    }

    GLint glformat;
    GLenum gltype;
};

// This method may dissapear in the future
inline void RenderToViewport(
    pangolin::Image<unsigned char>& image,
    const pangolin::GlPixFormat& fmt,
    bool flipx=false, bool flipy=false,
    bool linear_sampling = true
) {
    pangolin::GlTexture& tex = pangolin::TextureCache::I().GlTex(image.w, image.h, GL_RGBA, GL_RGBA, fmt.gltype);
    tex.Bind();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, linear_sampling ? GL_LINEAR : GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, linear_sampling ? GL_LINEAR : GL_NEAREST);
    tex.Upload(image.ptr,0,0, image.w, image.h, fmt.glformat, fmt.gltype);
    tex.RenderToViewport(pangolin::Viewport(0,0,image.w, image.h), flipx, flipy);
}

}

#endif // PANGOLIN_GLPANGOPIXFORMAT_H
