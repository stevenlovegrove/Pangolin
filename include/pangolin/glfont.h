/* This file is part of the Pangolin Project.
 * http://github.com/stevenlovegrove/Pangolin
 *
 * Copyright (c) 2013 Robert Castle, Gabe Sibley, Steven Lovegrove
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

#ifndef __GL_FONT_H__
#define __GL_FONT_H__

#include <pangolin/glinclude.h>
#include <pangolin/gl.h>
#include <pangolin/compat/memory.h>

#include <cstdio>
#include <cstdarg>
#include <string>
#include <map>
#include <vector>

namespace pangolin {

struct BitmapFontPadding
{
    int nTop;
    int nBottom;
    int nLeft;
    int nRight;
    
    BitmapFontPadding() : nTop(0), nBottom(0), nLeft(0), nRight(0) {}
};

struct BitmapFontSpacing
{
    int nTop;
    int nLeft;
    
    BitmapFontSpacing() : nTop(0), nLeft(0) {}
};

struct BitmapFontInfo
{
    std::string sName;
    int nSize;
    bool bBold;
    bool bItalic;
    std::string sCharset;
    bool bUnicode;
    int nStretchHeight;
    bool bSmooth;
    bool bAntiAliasing;
    BitmapFontPadding padding;
    BitmapFontSpacing spacing;
    int nOutline;
    
    BitmapFontInfo() : sName(""), nSize(0), bBold(false), bItalic(false),
        sCharset(""), bUnicode(false), nStretchHeight(0),
        bSmooth(false), bAntiAliasing(false), nOutline(0) {}
};

struct BitmapFontCommon
{
    enum ChannelType{ GLYPH = 0, OUTLINE = 1, GLYPHOUTLINE = 2, ZERO = 3, ONE = 4 };
    int nLineHeight;
    int nBase;
    int nScaleWidth;
    int nScaleHeight;
    int nPages;
    bool bPacked;
    ChannelType ctAlpha;
    ChannelType ctRed;
    ChannelType ctGreen;
    ChannelType ctBlue;
};

struct BitmapChar
{
    int x;
    int y;
    int width;
    int height;
    int xOffset;
    int yOffset;
    int xAdvance;
    int page;
    int channel;
    
    //list of proceeding characters where kerning of this char is needed.
    std::map< char, int > mKernings;
    BitmapChar() : x(0), y(0), width(0), height(0) {}
};

struct StrInfo {
    unsigned int width;                           // width of the string
    unsigned int height;                          //height of the string
    unsigned int baseline;                        //where the base line of the first line is from bottom right left
    unsigned int lineHeight;                      //how tall a line is (const per font)
    StrInfo() : width(0), height(0), baseline(0), lineHeight(0) {}
};


class GlFont
{
public:
    // Load font now (requires OpenGL context)
    bool LoadFontFromText( char* str_xml );
    bool LoadEmbeddedFont();
    bool LoadFontFromFile( const std::string& filename );
    
    // printf style function take position to print to as well
    void glPrintf(int x, int y, const char *fmt, ...);
    void glPrintf(int x, int y, const std::string fmt, ...){ glPrintf(x,y, fmt.c_str()); }
    
    /// Return information about how the string will be rendered
    const StrInfo StringInfo( std::string s ) const;
    unsigned int StringWidth( std::string s ) { return StringInfo( s ).width; }
    unsigned int StringHeight( std::string s ) { return StringInfo( s ).height; }
    unsigned int LineHeight() { return mCommon.nLineHeight; }
    
protected:
    void DrawChar(const BitmapChar & bc);
    void DrawString( int x, int y, std::string s );
    
    BitmapFontInfo mInfo;                            // info about the font
    BitmapFontCommon mCommon;                        // further font info
    std::vector<boostd::shared_ptr<GlTexture> > mvPages;             // the texture files that make up the font
    std::map< char, BitmapChar > mmCharacters;       // the list of characters in the font
};

}

#endif
