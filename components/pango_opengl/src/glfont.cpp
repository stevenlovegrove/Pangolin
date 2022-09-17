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

#include <locale>
#include <codecvt>

#include <pangolin/gl/glfont.h>
#include <pangolin/gl/glstate.h>
#include <pangolin/image/image_io.h>
#include <pangolin/utils/type_convert.h>
#include <pangolin/utils/file_utils.h>

#if !defined(HAVE_GLES) || defined(HAVE_GLES_2)
#include <pangolin/gl/glsl.h>
#endif

#define STB_TRUETYPE_IMPLEMENTATION
#define STBTT_STATIC

#if defined(__GNUC__)
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wunused-function"
#  include "stb_truetype.h"
#  pragma GCC diagnostic pop
#else
#  include "stb_truetype.h"
#endif

#define MAX_TEXT_LENGTH 500

namespace pangolin
{

// Hacked version of stbtt_FindGlyphIndex to extract valid codepoints in font
// Good reference for file format here https://developer.apple.com/fonts/TrueType-Reference-Manual/RM06/Chap6cmap.html
std::vector<std::pair<uint32_t,uint32_t>> GetCodepointRanges(const stbtt_fontinfo *info)
{
    std::vector<std::pair<uint32_t,uint32_t>> ranges;

    stbtt_uint8 *data = info->data;
    stbtt_uint32 index_map = info->index_map;

    stbtt_uint16 format = ttUSHORT(data + index_map + 0);
    if (format == 0) { // apple byte encoding
        stbtt_int32 bytes = ttUSHORT(data + index_map + 2);
        ranges.emplace_back(0, bytes-6);
    } else if (format == 6) {
        stbtt_uint32 first = ttUSHORT(data + index_map + 6);
        stbtt_uint32 count = ttUSHORT(data + index_map + 8);
        ranges.emplace_back(first, first+count);
    } else if (format == 2) {
        STBTT_assert(0); // @TODO: high-byte mapping for japanese/chinese/korean
    } else if (format == 4) { // standard mapping for windows fonts: binary search collection of ranges
        const stbtt_uint16 segcount = ttUSHORT(data+index_map+6) >> 1;

        // table ranges from [0 to segcount-1]
        stbtt_uint8 *endCodeTable = data + index_map + 14;
        stbtt_uint8 *startCodeTable = endCodeTable + 2*segcount + 2;

        for(size_t seg = 0; seg < segcount; ++seg) {
            const uint16_t startCode = ttUSHORT(startCodeTable + 2*seg);
            const uint16_t endCode = ttUSHORT(endCodeTable + 2*seg);

            if(endCode != 0xFFFF) {
                if(startCode < endCode) {
                    ranges.emplace_back(startCode,endCode+1);
                }
            }else{
                break;
            }
        }
    } else if (format == 12 || format == 13) {
        const stbtt_uint32 ngroups = ttULONG(data + index_map + 12);
        stbtt_uint8* range_table = data + index_map + 12 + 4;

        for(size_t group = 0; group < ngroups; ++group)
        {
            const uint32_t startCode = ttULONG(range_table + 4*(3*group));
            const uint32_t endCode = ttULONG(range_table + 4*(3*group+1));
            ranges.emplace_back(startCode,endCode+1);
        }
    } else{
        STBTT_assert(0);
    }

    return ranges;
}

std::string vformat(const char * format, va_list args)
{
  std::string result;
  va_list args_len;
  va_copy(args_len, args);

  const int len = vsnprintf(nullptr, 0, format, args_len);
  va_end(args_len);

  if (len > 0) {
    result.resize(len);
    // Guarenteed to write up to len only (+1 is left by vsnprintf for '\0')
    vsnprintf(result.data(), len+1, format, args);
    va_end(args);
  }

  return result;
}

std::string format(const char * format, ...)
{
  va_list args;
  va_start(args, format);
  const std::string s = vformat(format, args);
  va_end(args);
  return s;
}

GlFont::GlFont(const unsigned char* truetype_data, float pixel_height, int tex_w, int tex_h)
{
    InitialiseFont(truetype_data, pixel_height, tex_w, tex_h);
}

GlFont::GlFont(const std::string& filename, float pixel_height, int tex_w, int tex_h)
{
    const std::string file_contents = GetFileContents(filename);
    InitialiseFont(reinterpret_cast<const unsigned char*>(file_contents.data()), pixel_height, tex_w, tex_h);
}

GlFont::~GlFont()
{
}

void GlFont::InitialiseFont(const unsigned char* truetype_data, float pixel_height, int tex_w, int tex_h)
{
    font_height_px = pixel_height;
    font_max_width_px = 0;

    font_bitmap.Reinitialise(tex_w,tex_h);
    const int offset = 0;

    stbtt_fontinfo f;
    if (!stbtt_InitFont(&f, truetype_data, offset)) {
       throw std::runtime_error("Unable to initialise font: stbtt_InitFont failed.");
    }

    float scale = stbtt_ScaleForPixelHeight(&f, pixel_height);

    font_bitmap.Memset(0);
    int x = 1;
    int y = 1;
    int bottom_y = 1;

    const auto ranges = GetCodepointRanges(&f);

    for(const auto& r : ranges) {
        for(uint32_t codepoint=r.first; codepoint < r.second; ++codepoint) {
            int advance, lsb, x0,y0,x1,y1,gw,gh;
            int g = stbtt_FindGlyphIndex(&f, codepoint);
            stbtt_GetGlyphHMetrics(&f, g, &advance, &lsb);
            stbtt_GetGlyphBitmapBox(&f, g, scale,scale, &x0,&y0,&x1,&y1);
            gw = x1-x0;
            gh = y1-y0;
            font_max_width_px = std::max(font_max_width_px, (float)gw);

            if (x + gw + 1 >= tex_w)
               y = bottom_y, x = 1; // advance to next row
            if (y + gh + 1 >= tex_h) // check if it fits vertically AFTER potentially moving to next row
               throw std::runtime_error("Unable to initialise font: run out of texture pixel space.");
            STBTT_assert(x+gw < tex_w);
            STBTT_assert(y+gh < tex_h);
            stbtt_MakeGlyphBitmap(&f, font_bitmap.RowPtr(y)+x, gw,gh, font_bitmap.pitch, scale, scale, g);

            // Adjust offset for edges of pixels
            chardata.try_emplace(codepoint, tex_w,tex_h, x, y, gw, gh, scale*advance, x0 -0.5f, -y0 -0.5f);

            x = x + gw + 1;
            if (y+gh+1 > bottom_y)
               bottom_y = y+gh+1;
        }
    }

    // This could be a nasty slow loop...
    for(const auto& r1 : ranges) {
        for(uint32_t cp1=r1.first; cp1 < r1.second; ++cp1) {
            for(const auto& r2 : ranges) {
                for(uint32_t cp2=r2.first; cp2 < r2.second; ++cp2) {
                    kern_table[codepointpair_t(cp1,cp2)] = scale * stbtt_GetCodepointKernAdvance(&f,cp1,cp2);
                }
            }
        }
    }
}

void GlFont::InitialiseGlTexture()
{
    if(font_bitmap.IsValid()) {
        mTex.Reinitialise(font_bitmap.w, font_bitmap.h, GL_ALPHA, true, 0, GL_ALPHA,GL_UNSIGNED_BYTE, font_bitmap.ptr);
        font_bitmap.Deallocate();
    }
}

GlText GlFont::Text( const char* format, ... )
{
    va_list args;
    va_start(args, format);
    const std::string s = vformat(format, args);
    va_end(args);
    return Text(s);
}

GlText GlFont::Text(const std::string& utf8 )
{
    const std::u32string utf32 = std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t>{}.from_bytes(utf8);

    if(!mTex.IsValid()) InitialiseGlTexture();

    GlText ret(mTex);
    ret.str = utf8;

    char32_t last_c = '\0';

    for(char32_t c : utf32)
    {
        const auto it = chardata.find(c);
        if(it != chardata.end()) {
            const GlChar& ch = it->second;

            // Kerning
            if(last_c) {
                codepointpair_t key(last_c, c);
                const auto kit = kern_table.find(key);
                const GLfloat kern = (kit != kern_table.end()) ? kit->second : font_max_width_px;
                ret.AddSpace(kern);
            }

            ret.Add(' ', ch);
            last_c = c;
        }else{
            // codepoint doesn't exists in font
            // TODO: use some symbol such as '?'?
        }
    }

    return ret;
}

}
