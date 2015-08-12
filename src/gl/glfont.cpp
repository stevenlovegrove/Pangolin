/* This file is part of the Pangolin Project.
 * http://github.com/stevenlovegrove/Pangolin
 *
 * Copyright (c) 2013 Steven Lovegrove, Robert Castle, Gabe Sibley
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

#include <pangolin/gl/glfont.h>
#include <pangolin/gl/glstate.h>
#include <pangolin/image/image_io.h>
#include <pangolin/utils/type_convert.h>

#if !defined(HAVE_GLES) || defined(HAVE_GLES_2)
#include <pangolin/gl/glsl.h>
#endif

#include <pangolin/utils/xml/rapidxml.hpp>
#include <pangolin/utils/xml/rapidxml_utils.hpp>

#include <stdlib.h>
#include <cstring>
#include <fstream>

// Include the default font directly
#include "glfontdata.h"

#define MAX_TEXT_LENGTH 500

namespace pangolin
{

void LoadGlImage(GlTexture& tex, const std::string& filename, bool sampling_linear)
{
    const GLenum chtypes[] = {
        GL_ALPHA, GL_LUMINANCE_ALPHA,
        GL_RGB, GL_RGBA
    };
    TypedImage img = LoadImage(filename);
    const GLint format  = chtypes[img.fmt.channels];
    const GLint imgtype = GL_UNSIGNED_BYTE;
    tex.Reinitialise((GLint)img.w, (GLint)img.h, format, sampling_linear, 0, format, imgtype, img.ptr );
    img.Dealloc();
}

GlFont& GlFont::I()
{
    static GlFont s_font;
    return s_font;
}

bool GlFont::LoadFontFromText(char* xml_text)
{
    rapidxml::xml_document<> doc;
    doc.parse<0>(xml_text);
    
    rapidxml::xml_node<>* node_font = doc.first_node("font");
    if(node_font) {
        rapidxml::xml_node<>* node_info   = node_font->first_node("info");
        rapidxml::xml_node<>* node_common = node_font->first_node("common");
        rapidxml::xml_node<>* node_pages  = node_font->first_node("pages");
        rapidxml::xml_node<>* node_chars  = node_font->first_node("chars");
        rapidxml::xml_node<>* node_kerns  = node_font->first_node("kernings");
        
        if(node_info && node_common && node_chars) {
            sName          = node_info->first_attribute_value<std::string>("face");
            sCharset       = node_info->first_attribute_value<std::string>("charset");
            nSize          = node_info->first_attribute_value<int>("size");
            nStretchHeight = node_info->first_attribute_value<int>("stretchH");
            nOutline       = node_info->first_attribute_value<int>("outline");
            bBold          = node_info->first_attribute_value<bool>("bold");
            bItalic        = node_info->first_attribute_value<bool>("italic");
            bUnicode       = node_info->first_attribute_value<bool>("unicode");
            bSmooth        = node_info->first_attribute_value<bool>("smooth");
            bAntiAliasing  = node_info->first_attribute_value<bool>("aa");
            
            nBase        = node_common->first_attribute_value<int>("base");
            nScaleWidth  = node_common->first_attribute_value<int>("scaleW");
            nScaleHeight = node_common->first_attribute_value<int>("scaleH");
            nLineHeight  = node_common->first_attribute_value<int>("lineHeight");
            nPages       = node_common->first_attribute_value<int>("pages");
            
            if(node_pages) {
                for( rapidxml::xml_node<>* xml_page = node_pages->first_node();
                     xml_page; xml_page = xml_page->next_sibling() )
                {
                    const std::string filename = xml_page->first_attribute_value<std::string>( "file" );
                    LoadGlImage(mTex, filename, false );
                    glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
                }
            }else{
                mTex.Reinitialise(nScaleWidth, nScaleHeight, GL_ALPHA,false,0,GL_ALPHA,GL_UNSIGNED_BYTE,font_image_data );
                glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
            }
            
            if( nPages < 1) {
                std::cerr << "Incorrect number of font pages loaded." << std::endl;
            }
            
            for( rapidxml::xml_node<>* xml_char = node_chars->first_node();
                 xml_char; xml_char = xml_char->next_sibling() )
            {
                char id      = xml_char->first_attribute_value<int>("id");
                int x        = xml_char->first_attribute_value<int>("x");
                int y        = xml_char->first_attribute_value<int>("y");
                int width    = xml_char->first_attribute_value<int>("width");
                int height   = xml_char->first_attribute_value<int>("height");
                int xOffset  = xml_char->first_attribute_value<int>("xoffset");
                int yOffset  = xml_char->first_attribute_value<int>("yoffset");
                int xAdvance = xml_char->first_attribute_value<int>("xadvance");
                int page     = xml_char->first_attribute_value<int>("page");
                
                if(page != 0) {
                    std::cerr << "Multi-page font not supported" << std::endl;
                    page = 0;
                }
                
                mmCharacters[id] = GlChar(nScaleWidth, nScaleHeight,
                                          x,y,width, height,xAdvance,
                                          (GLfloat)xOffset, (GLfloat)(nBase - yOffset) );
            }
            
            if(node_kerns) {
                for( rapidxml::xml_node<>* xml_kern = node_kerns->first_node();
                     xml_kern; xml_kern = xml_kern->next_sibling() )
                {
                    char first  = xml_kern->first_attribute_value<char>("first");
                    char second = xml_kern->first_attribute_value<char>("second");
                    int amount  = xml_kern->first_attribute_value<int>("amount");
                    mmCharacters[second].SetKern(first,amount);
                }
            }
        }
    }
    return false;
}

bool GlFont::LoadFontFromFile( const std::string& filename )
{
    try{
        rapidxml::file<> xmlFile(filename.c_str());
        return LoadFontFromText( xmlFile.data() );
    }catch(std::exception) {
        return false;
    }
}

bool GlFont::LoadEmbeddedFont()
{
    // Include an extra byte for the terminating NULL
    char* str = new char[font_xml_data.size() + 1];
    std::memcpy(str, font_xml_data.c_str(), font_xml_data.size());
    str[font_xml_data.size()] = '\0';
    const bool success = LoadFontFromText(str);
    delete[] str;
    return success;
}

void GlFont::UnloadFont()
{
    mTex.Delete();
    mmCharacters.clear();
}

GlText GlFont::Text( const char* fmt, ... )
{
    if(!mmCharacters.size()) LoadEmbeddedFont();
    
    GlText ret(mTex);
    
    char text[MAX_TEXT_LENGTH];          
    va_list ap;
    
    if( fmt != NULL ) {
        va_start( ap, fmt );
        vsnprintf( text, MAX_TEXT_LENGTH, fmt, ap );
        va_end( ap );
        
        const size_t len = strlen(text);
        for(size_t i=0; i < len; ++i) {
            const char c = text[i];
            std::map< char, GlChar >::const_iterator it = mmCharacters.find( c );
            if(it != mmCharacters.end()) {
                ret.Add(c, it->second);
            }
        }
    }
    return ret;
}

}
