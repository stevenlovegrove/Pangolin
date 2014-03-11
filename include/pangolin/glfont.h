/* This file is part of the Pangolin Project.
 * http://github.com/stevenlovegrove/Pangolin
 *
 * Copyright (c) 2013 Robert Castle, Steven Lovegrove, Gabe Sibley
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

#ifndef PANGOLIN_GLFONT_H
#define PANGOLIN_GLFONT_H

#include <pangolin/gltext.h>

#include <cstdio>
#include <cstdarg>

namespace pangolin {

class PANGOLIN_EXPORT GlFont
{
public:
    // Singleton instance if requested.
    static GlFont& I();
    
    // Load font now (requires OpenGL context)
    bool LoadFontFromText( char* str_xml );
    bool LoadEmbeddedFont();
    bool LoadFontFromFile( const std::string& filename );
    void UnloadFont();

    // Generate renderable GlText object from this font.
    GlText Text( const char* fmt, ... );
    
protected:
    std::string sName;
    int nSize;
    bool bBold;
    bool bItalic;
    std::string sCharset;
    bool bUnicode;
    int nStretchHeight;
    bool bSmooth;
    bool bAntiAliasing;
    int nOutline;
    int nLineHeight;
    int nBase;
    int nScaleWidth;
    int nScaleHeight;
    int nPages;
    
    GlTexture mTex;
    std::map< char, GlChar > mmCharacters;
};

}

#endif // PANGOLIN_GLFONT_H
