/* This file is part of the Pangolin Project.
 * http://github.com/stevenlovegrove/Pangolin
 *
 * Copyright (c) 2013 Steven Lovegrove
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

#ifndef PANGOLIN_GLTEXT_H
#define PANGOLIN_GLTEXT_H

#include <pangolin/glinclude.h>
#include <pangolin/gl.h>
#include <pangolin/glchar.h>

#include <vector>
#include <string>

namespace pangolin {

class GlText
{
public:
    GlText(const GlTexture& font_tex);
    
    void Add(const GlChar& c);

    void Draw();
    
    const std::string& Text() const
    {
        return str;
    }
    
    int Width() const {
        return width;
    }
    
    int Height() const {
        return ymax - ymin;
    }

protected:
    const GlTexture* tex;
    std::string str;
    unsigned int width;
    unsigned int ymin;
    unsigned int ymax;
    
    std::vector<XYUV> vs;
};

}

#endif // PANGOLIN_GLTEXT_H
