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

#include <pangolin/glfont.h>
#include <pangolin/image_load.h>
#include <pangolin/type_convert.h>

#include <pangolin/xml/rapidxml.hpp>
#include <pangolin/xml/rapidxml_utils.hpp>

#include <stdlib.h>
#include <cstring>
#include <fstream>

// Include the default font directly
#include "glfontdata.h"

#define MAX_TEXT_LENGTH 500

namespace pangolin
{

std::istream& operator>>(std::istream& is, BitmapFontPadding& pad) {
    is >> pad.nTop; is.get(); is >> pad.nRight; is.get();
    is >> pad.nBottom; is.get(); is >> pad.nLeft;
    return is;
}

std::istream& operator>>(std::istream& is, BitmapFontSpacing& space) {
    is >> space.nTop; is.get(); is >> space.nLeft;
    return is;
}

std::istream& operator>>(std::istream& is, BitmapFontCommon::ChannelType& chtype) {
    int ict;
    is >> ict;
    chtype = (BitmapFontCommon::ChannelType) ict;
    return is;
}

void GlFont::glPrintf(int x, int y, const char *fmt, ...)
{
    char text[MAX_TEXT_LENGTH];          
    va_list ap;
    
    if( fmt != NULL ) {
        va_start( ap, fmt );
        vsnprintf( text, MAX_TEXT_LENGTH, fmt, ap );
        va_end( ap );
        
        glDisable(GL_DEPTH_TEST); //causes text not to clip with geometry
        DrawString(x, y, text);
        glEnable(GL_DEPTH_TEST);
    }
}

void LoadGlImage(GlTexture& tex, const std::string& filename, bool sampling_linear)
{
    const GLenum chtypes[] = {
        GL_ALPHA, GL_LUMINANCE_ALPHA,
        GL_RGB, GL_RGBA
    };
    TypedImage img = LoadImage(filename);
    const GLint format  = chtypes[img.fmt.channels];
    const GLint imgtype = GL_UNSIGNED_BYTE;
    tex.Reinitialise(img.w, img.h, format, sampling_linear, 0, format, imgtype, img.ptr );
    img.Dealloc();
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
            mInfo.sName          = node_info->first_attribute_value<std::string>("face");
            mInfo.sCharset       = node_info->first_attribute_value<std::string>("charset");
            mInfo.nSize          = node_info->first_attribute_value<int>("size");
            mInfo.nStretchHeight = node_info->first_attribute_value<int>("stretchH");
            mInfo.nOutline       = node_info->first_attribute_value<int>("outline");
            mInfo.bBold          = node_info->first_attribute_value<bool>("bold");
            mInfo.bItalic        = node_info->first_attribute_value<bool>("italic");
            mInfo.bUnicode       = node_info->first_attribute_value<bool>("unicode");
            mInfo.bSmooth        = node_info->first_attribute_value<bool>("smooth");
            mInfo.bAntiAliasing  = node_info->first_attribute_value<bool>("aa");
            mInfo.padding        = node_info->first_attribute_value("padding", 0, BitmapFontPadding() );
            mInfo.spacing        = node_info->first_attribute_value("spacing", 0, BitmapFontSpacing() );
            
            mCommon.nBase        = node_common->first_attribute_value<int>("base");
            mCommon.nScaleWidth  = node_common->first_attribute_value<int>("scaleW");
            mCommon.nScaleHeight = node_common->first_attribute_value<int>("scaleH");
            mCommon.nLineHeight  = node_common->first_attribute_value<int>("lineHeight");
            mCommon.nPages       = node_common->first_attribute_value<int>("pages");
            mCommon.bPacked      = node_common->first_attribute_value<bool>("packed");
            mCommon.bPacked      = node_common->first_attribute_value("alphaChnl", 0, BitmapFontCommon::GLYPH );
            mCommon.bPacked      = node_common->first_attribute_value("redChnl", 0, BitmapFontCommon::GLYPH );
            mCommon.bPacked      = node_common->first_attribute_value("greenChnl", 0, BitmapFontCommon::GLYPH );
            mCommon.bPacked      = node_common->first_attribute_value("blueChnl", 0, BitmapFontCommon::GLYPH );
            
            if(node_pages) {
                for( rapidxml::xml_node<>* xml_page = node_pages->first_node();
                     xml_page; xml_page = xml_page->next_sibling() )
                {
                    const std::string filename = xml_page->first_attribute_value<std::string>( "file" );
                    GlTexture* tex = new GlTexture();
                    LoadGlImage(*tex, filename, false );
                    glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
                    mvPages.push_back( boostd::shared_ptr<GlTexture>(tex) );
                }
            }else{
                GlTexture* tex = new GlTexture(mCommon.nScaleWidth, mCommon.nScaleHeight, GL_ALPHA,false,0,GL_ALPHA,GL_UNSIGNED_BYTE,font_image_data );
                glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
                mvPages.push_back( boostd::shared_ptr<GlTexture>(tex) );
            }
            
            for( rapidxml::xml_node<>* xml_char = node_chars->first_node();
                 xml_char; xml_char = xml_char->next_sibling() )
            {
                BitmapChar bc;
                char id     = xml_char->first_attribute_value<int>("id");
                bc.x        = xml_char->first_attribute_value<int>("x");
                bc.y        = xml_char->first_attribute_value<int>("y");
                bc.width    = xml_char->first_attribute_value<int>("width");
                bc.height   = xml_char->first_attribute_value<int>("height");
                bc.xOffset  = xml_char->first_attribute_value<int>("xoffset");
                bc.yOffset  = xml_char->first_attribute_value<int>("yoffset");
                bc.xAdvance = xml_char->first_attribute_value<int>("xadvance");
                bc.page     = xml_char->first_attribute_value<int>("page");
                bc.channel  = xml_char->first_attribute_value<int>("chnl");                
                mmCharacters[id] = bc;
            }
            
            if(node_kerns) {
                for( rapidxml::xml_node<>* xml_kern = node_kerns->first_node();
                     xml_kern; xml_kern = xml_kern->next_sibling() )
                {
                    char first  = xml_kern->first_attribute_value<char>("first");
                    char second = xml_kern->first_attribute_value<char>("second");
                    int amount  = xml_kern->first_attribute_value<int>("amount");
                    mmCharacters[second].mKernings[first] = amount;
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
    char* str = new char[font_xml_data.size()];
    strcpy( str, font_xml_data.c_str() );
    const bool success = LoadFontFromText(str);
    delete[] str;
    return success;
}

void GlFont::DrawChar(const BitmapChar & bc)
{
    GlTexture& page = *(mvPages[bc.page]);
    const GLfloat w = bc.width;
    const GLfloat h = bc.height;
    const GLfloat ox = bc.xOffset;
    const GLfloat oy = bc.yOffset;
    const GLfloat pW = page.width;
    const GLfloat pH = page.height;
    
    GLfloat u =  ((GLfloat)bc.x - 0.5f ) / pW;
    GLfloat v =  ((GLfloat)bc.y - 0.5f ) / pH;
    GLfloat u2 = u + (w + 0.5f)/ pW;
    GLfloat v2 = v + (h + 0.5f)/ pH;
    
    GLfloat y = (double)mCommon.nBase;
    
    GLfloat sq_vert[] = { ox, y-oy,  ox, y-h-oy,  w+ox, y-h-oy,  w+ox, y-oy };
    glVertexPointer(2, GL_FLOAT, 0, sq_vert);
    glEnableClientState(GL_VERTEX_ARRAY);   
    
    GLfloat sq_tex[]  = { u,v,  u,v2,  u2,v2,  u2,v };
    glTexCoordPointer(2, GL_FLOAT, 0, sq_tex);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    page.Bind();
    glEnable(GL_TEXTURE_2D);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glDisable(GL_TEXTURE_2D);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    
    glTranslatef(bc.xAdvance + 2*mInfo.nOutline, 0, 0);
}

void GlFont::DrawString( int x, int y, std::string s )
{
    // Make sure we have at least one font
    if(!mmCharacters.size()) LoadEmbeddedFont();
    
    glDisable( GL_DEPTH_TEST );
    glPushMatrix();
    glTranslatef(x,y,0);
    glEnable( GL_TEXTURE_2D );
    
    for( size_t i = 0; i < s.length(); i++ ) {
        const char c = s[i];
        std::map< char, BitmapChar >::const_iterator it;
        it = mmCharacters.find( c );
        
        if( it == mmCharacters.end() )  {
            continue;
        }
        
        const BitmapChar & bc = (*it).second;
        
        mvPages[bc.page]->Bind();
        
        //kerning
        if( i > 0 ) {
            std::map< char, int >::const_iterator k;
            k = bc.mKernings.find( s[i - 1] );
            if( k != bc.mKernings.end() )  {
                glTranslatef( (*k).second, 0, 0);
            }
        }
        
        DrawChar( bc );
    }
    
    glDisable(GL_TEXTURE_2D);
    
    glPopMatrix();
    glEnable( GL_DEPTH_TEST );
}

const StrInfo GlFont::StringInfo( std::string s ) const
{
    StrInfo si;
    
    if( s.empty() ) {
        return si;
    }
    
    int yMin = 1000, yMax = 0;
    std::map< char, BitmapChar >::const_iterator it;
    
    for( size_t i = 0; i < s.size(); i++ ) {
        it = mmCharacters.find( s[i] );
        if( it != mmCharacters.end() )  {
            const BitmapChar & c = (*it).second;
            
            si.width += c.xAdvance + 2*mInfo.nOutline;
            
            std::map< char, int >::const_iterator k;
            k = c.mKernings.find( s[i-1] );
            if( k != c.mKernings.end() )  {
                si.width += (*k).second;
            }
            
            yMin = std::min( yMin, c.yOffset );
            yMax = std::max( yMax, c.yOffset + c.height);
        }
    }
    
    si.height = yMax - yMin;
    si.baseline = si.height - (mCommon.nBase - yMin);
    si.lineHeight = mCommon.nLineHeight;
    
    return si;
}

}
