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

#include <pangolin/view.h>
#include <pangolin/display.h>

#include <pangolin/glfont.h>
#include <pangolin/glstate.h>
#include <pangolin/image_load.h>
#include <pangolin/type_convert.h>
#include <pangolin/glsl.h>

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

GlChar::GlChar()
{
}

GlChar::GlChar(int tw, int th, int x, int y, int w, int h, int advance, GLfloat ox, GLfloat oy)
    : x_step(advance)
{
    const GLfloat u = (x-0.5f) / tw;
    const GLfloat v = (y-0.5f) / th;
    const GLfloat u2 = u + (w + 0.5f) / tw;
    const GLfloat v2 = v + (h + 0.5f) / th;
    
    // Setup u,v tex coords
    vs[0] = XYUV(ox, oy,     u,v );
    vs[1] = XYUV(ox, oy-h,   u,v2 );
    vs[2] = XYUV(w+ox, oy-h, u2,v2 );
    vs[3] = XYUV(w+ox, oy,   u2,v );
}

void GlChar::Draw() const
{
    glVertexPointer(2, GL_FLOAT, sizeof(XYUV), &vs[0].x);
    glEnableClientState(GL_VERTEX_ARRAY);   
    glTexCoordPointer(2, GL_FLOAT, sizeof(XYUV), &vs[0].tu);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnable(GL_TEXTURE_2D);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glDisable(GL_TEXTURE_2D);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}

void GlChar::SetKern(char c, int kern)
{
    mKernings[c] = kern;
}

int GlChar::Kern(char c) const
{
    std::map< char, int >::const_iterator k = mKernings.find(c);
    if(k != mKernings.end())  return k->second;
    return 0;
}

GlText::GlText()
    : tex(NULL), width(0),
      ymin(std::numeric_limits<int>::max()),
      ymax(std::numeric_limits<int>::min())
{
    
}

GlText::GlText(const GlTexture& font_tex)
    : tex(&font_tex), width(0),
      ymin(std::numeric_limits<int>::max()),
      ymax(std::numeric_limits<int>::min())
{
}

void GlText::Add(const GlChar& c)
{
    int k = 0;
    int x = width;
    
    if(str.size()) {
        k = c.Kern(str[str.size()-1]);
        x += k;
    }
    
    vs.push_back(c.GetVert(0) + x);
    vs.push_back(c.GetVert(1) + x);
    vs.push_back(c.GetVert(2) + x);
    vs.push_back(c.GetVert(0) + x);
    vs.push_back(c.GetVert(2) + x);
    vs.push_back(c.GetVert(3) + x);
    
    width = x + c.StepX();        
}

void GlText::DrawGlSl()
{
    if(vs.size() && tex) {
        glEnableVertexAttribArray(pangolin::DEFAULT_LOCATION_POSITION);
        glEnableVertexAttribArray(pangolin::DEFAULT_LOCATION_TEXCOORD);

        glVertexAttribPointer(pangolin::DEFAULT_LOCATION_POSITION, 2, GL_FLOAT, GL_FALSE, sizeof(XYUV), &vs[0].x);
        glVertexAttribPointer(pangolin::DEFAULT_LOCATION_TEXCOORD, 2, GL_FLOAT, GL_FALSE, sizeof(XYUV), &vs[0].tu);

        tex->Bind();
        glEnable(GL_TEXTURE_2D);
        glDrawArrays(GL_TRIANGLES, 0, vs.size() );
        glDisable(GL_TEXTURE_2D);

        glDisableVertexAttribArray(pangolin::DEFAULT_LOCATION_POSITION);
        glDisableVertexAttribArray(pangolin::DEFAULT_LOCATION_TEXCOORD);
    }
}

void GlText::Draw()
{
    if(vs.size() && tex) {
        glVertexPointer(2, GL_FLOAT, sizeof(XYUV), &vs[0].x);
        glEnableClientState(GL_VERTEX_ARRAY);   
        glTexCoordPointer(2, GL_FLOAT, sizeof(XYUV), &vs[0].tu);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        tex->Bind();
        glEnable(GL_TEXTURE_2D);
        glDrawArrays(GL_TRIANGLES, 0, vs.size() );
        glDisable(GL_TEXTURE_2D);
        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    }
}

void GlText::Draw(GLfloat x, GLfloat y, GLfloat z)
{
    // find object point (x,y,z)' in pixel coords
    GLdouble projection[16];
    GLdouble modelview[16];
    GLint    view[4];
    GLdouble scrn[3];

#ifdef HAVE_GLES_2
    std::copy(glEngine().projection.top().m, glEngine().projection.top().m+16, projection);
    std::copy(glEngine().modelview.top().m, glEngine().modelview.top().m+16, modelview);
#else
    glGetDoublev(GL_PROJECTION_MATRIX, projection );
    glGetDoublev(GL_MODELVIEW_MATRIX, modelview );
#endif
    glGetIntegerv(GL_VIEWPORT, view );
    
    pangolin::glProject(x, y, z, modelview, projection, view,
        scrn, scrn + 1, scrn + 2);
    
    DisplayBase().Activate();
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();    
    glOrtho(0, DisplayBase().v.w, 0, DisplayBase().v.h, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    glTranslatef((int)scrn[0],(int)scrn[1],scrn[2]);
    Draw();

    // Restore viewport
    glViewport(view[0],view[1],view[2],view[3]);
    
    // Restore modelview / project matrices
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();    
}

// Render at (x,y) in window coordinates.
void GlText::DrawWindow(GLfloat x, GLfloat y, GLfloat z)
{
    // Backup viewport
    GLint    view[4];
    glGetIntegerv(GL_VIEWPORT, view );
        
    DisplayBase().Activate();
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();    
    glOrtho(0, DisplayBase().v.w, 0, DisplayBase().v.h, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    glTranslatef((int)x,(int)y,z);
    Draw();

    // Restore viewport
    glViewport(view[0],view[1],view[2],view[3]);
    
    // Restore modelview / project matrices
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();    
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
                                          xOffset,nBase - yOffset);
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
    strcpy( str, font_xml_data.c_str() );
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
        
        int len = strlen(text);
        for(int i=0; i < len; ++i) {
            const char c = text[i];
            std::map< char, GlChar >::const_iterator it = mmCharacters.find( c );
            if(it != mmCharacters.end()) {
                ret.Add(it->second);
            }
        }
    }
    return ret;
}

}
