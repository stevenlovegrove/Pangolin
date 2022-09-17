/* This file is part of the Pangolin Project.
 * http://github.com/stevenlovegrove/Pangolin
 *
 * Copyright (c) 2015 Steven Lovegrove
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

#include <pangolin/gl/gltext.h>

#include <cstdio>
#include <cstdarg>
#include <unordered_map>

namespace pangolin {

class PANGOLIN_EXPORT GlFont
{
public:
    // Load GL Font data. Delay uploading as texture until first use.
    GlFont(const unsigned char* ttf_buffer, float pixel_height, int tex_w=512, int tex_h=512);
    GlFont(const std::string& filename, float pixel_height, int tex_w=512, int tex_h=512);

    virtual ~GlFont();

    // Generate renderable GlText object from this font.
    GlText Text( const char* fmt, ... );

    // Utf8 encoded string
    GlText Text( const std::string& utf8 );

    inline float Height() const {
        return font_height_px;
    }
    inline float MaxWidth() const {
        return font_max_width_px;
    }

protected:
    void InitialiseFont(const unsigned char* ttf_buffer, float pixel_height, int tex_w, int tex_h);

    // This can only be called once GL context is initialised
    void InitialiseGlTexture();

    float font_height_px;
    float font_max_width_px;

    ManagedImage<unsigned char> font_bitmap;
    GlTexture mTex;

    using codepoint_t = uint32_t;
    using codepointpair_t = std::pair<codepoint_t, codepoint_t>;

    std::map<codepoint_t, GlChar> chardata;
    std::map<codepointpair_t, GLfloat> kern_table;
};

}
