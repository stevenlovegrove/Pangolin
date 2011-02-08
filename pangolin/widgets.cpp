/* This file is part of the Pangolin Project.
 * http://github.com/stevenlovegrove/Pangolin
 *
 * Copyright (c) 2011 Steven Lovegrove
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

#include "widgets.h"

#include <iostream>
#include <iomanip>
#include "display_internal.h"

using namespace std;

namespace pangolin
{

extern __thread PangolinGl* context;

const static int border = 1;
const static int tab_w = 15;
const static int tab_h = 20;
const static int tab_p = 5;
const static float colour_s1[4] = {0.2, 0.2, 0.2, 1.0};
const static float colour_s2[4] = {0.6, 0.6, 0.6, 1.0};
const static float colour_bg[4] = {0.9, 0.9, 0.9, 1.0};
const static float colour_fg[4] = {1.0, 1.0 ,1.0, 1.0};
const static float colour_tx[4] = {0.0, 0.0, 0.0, 1.0};
const static float colour_hl[4] = {0.9, 0.9, 0.9, 1.0};
const static float colour_dn[4] = {1.0, 0.7 ,0.7, 1.0};
static void* font = GLUT_BITMAP_HELVETICA_12;
static int text_height = 8; //glutBitmapHeight(font) * 0.7;
static float cb_height = text_height * 1.5;

void glRect(Viewport v)
{
    glRectf(v.l,v.t()-1,v.r()-1,v.b);
}

void glRect(Viewport v, int inset)
{
    glRectf(v.l+inset,v.t()-inset-1,v.r()-inset-1,v.b+inset);
}

void DrawShadowRect(Viewport& v)
{
    glColor4fv(colour_s2);
    glBegin(GL_LINE_STRIP);
    glVertex2f(v.l,v.b);
    glVertex2f(v.l,v.t());
    glVertex2f(v.r(),v.t());
    glVertex2f(v.r(),v.b);
    glVertex2f(v.l,v.b);
    glEnd();
}

void DrawShadowRect(Viewport& v, bool pushed)
{
    glColor4fv(pushed ? colour_s1 : colour_s2);
    glBegin(GL_LINE_STRIP);
    glVertex2f(v.l,v.b);
    glVertex2f(v.l,v.t());
    glVertex2f(v.r(),v.t());
    glEnd();

    glColor3fv(pushed ? colour_s2 : colour_s1);
    glBegin(GL_LINE_STRIP);
    glVertex2f(v.r(),v.t());
    glVertex2f(v.r(),v.b);
    glVertex2f(v.l,v.b);
    glEnd();
}

Panel::Panel()
{
    handler = &StaticHandler;
    layout = LayoutVertical;
}

Panel::Panel(const std::string& auto_register_var_prefix)
{
    handler = &StaticHandler;
    layout = LayoutVertical;
    RegisterNewVarCallback(&Panel::AddVariable,(void*)this,auto_register_var_prefix);
}

void Panel::AddVariable(void* data, const std::string& name, _Var& var)
{
    Panel* thisptr = (Panel*)data;

    const string& title = var.meta_friendly;

    if( var.type_name == typeid(bool).name() )
    {
        thisptr->views.push_back(
                    var.meta_flags ? (View*)new Checkbox(title,var) : (View*)new Button(title,var)
                                     );
    }else if( var.type_name == typeid(double).name() || var.type_name == typeid(int).name() )
    {
        thisptr->views.push_back( new Slider(title,var) );
    }

}

void Panel::Render()
{
    OpenGlRenderState::ApplyWindowCoords();
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_SCISSOR_TEST);

    glColor4fv(colour_s2);
    glRect(v);
    glColor4fv(colour_bg);
    glRect(vinside);

    RenderChildren();
}

void Panel::ResizeChildren()
{
    vinside = v.Inset(border,border);
    View::ResizeChildren();
}


View& CreatePanel(const std::string& name)
{
    Panel* v = new Panel(name);
    context->all_views[name] = v;
    context->base.views.push_back(v);
    return *v;
}

Button::Button(string title, _Var& tv)
    : Var<bool>(tv), title(title), down(false)
{
    top = 1.0; bottom = -20;
    left = 0; right = 1.0;
    hlock = LockLeft;
    vlock = LockBottom;
    handler = this;
    text_width = glutBitmapLength(font,(unsigned char*)title.c_str());
}

void Button::Mouse(View&, MouseButton button, int x, int y, bool pressed, int mouse_state)
{
    down = pressed;
    if( !pressed ) a->Set(!a->Get());
}

void Button::Render()
{
    DrawShadowRect(v, down);
    glColor4fv(colour_fg );
    glRect(vinside);
    glColor4fv(colour_tx);
    glRasterPos2f(raster[0],raster[1]-down);
    glutBitmapString(font,(unsigned char*)title.c_str());
}

void Button::ResizeChildren()
{
    raster[0] = v.l + (v.w-text_width)/2.0;
    raster[1] = v.b + (v.h-text_height)/2.0;
    vinside = v.Inset(border);
}

Checkbox::Checkbox(std::string title, _Var& tv)
    :Var<bool>(tv), title(title)
{
    top = 1.0; bottom = -20;
    left = 0; right = 1.0;
    hlock = LockLeft;
    vlock = LockBottom;
    handler = this;
}

void Checkbox::Mouse(View&, MouseButton button, int x, int y, bool pressed, int mouse_state)
{
    if( pressed )
        a->Set(!a->Get());
}

void Checkbox::ResizeChildren()
{
    raster[0] = v.l + cb_height + 4;
    raster[1] = v.b + (v.h-text_height)/2.0;
    const float h = v.h;
    const float t = (h-cb_height) / 2.0;
    vcb = Viewport(v.l,v.b+t,cb_height,cb_height);
}

void Checkbox::Render()
{
    const bool val = a->Get();

    DrawShadowRect(vcb, val);
    if( val )
    {
        glColor4fv(colour_dn);
        glRect(vcb);
    }
    glColor4fv(colour_tx);
    glRasterPos2fv( raster );
    glutBitmapString(font,(unsigned char*)title.c_str());
}


Slider::Slider(std::string title, _Var& tv)
    :Var<double>(tv), title(title+":"), lock_bounds(true)
{
    top = 1.0; bottom = -20;
    left = 0; right = 1.0;
    hlock = LockLeft;
    vlock = LockBottom;
    handler = this;
}

void Slider::Keyboard(View&, unsigned char key, int x, int y, bool pressed){

    if( pressed && var->meta_range[0] < var->meta_range[1] )
    {
        const double val = a->Get();
        if(key=='-'){
            a->Set( max(var->meta_range[0],min(var->meta_range[1],val - var->meta_increment) ) );
        }else if(key == '='){
            a->Set( max(var->meta_range[0],min(var->meta_range[1],val + var->meta_increment) ) );
        }else if(key == 'r'){
            Reset();
        }
    }
}

void Slider::Mouse(View& view, MouseButton button, int x, int y, bool pressed, int mouse_state)
{
    if(pressed)
    {
        // Wheel
        if( button == MouseWheelUp || button == MouseWheelDown )
        {
            // Change scale around current value
            const double frac = max(0.0,min(1.0,(double)(x - v.l)/(double)v.w));
            const double val = frac * (var->meta_range[1] - var->meta_range[0]) + var->meta_range[0];
            const double scale = (button == MouseWheelUp ? 1.2 : 1.0 / 1.2 );
            var->meta_range[1] = val + (var->meta_range[1] - val)*scale;
            var->meta_range[0] = val - (val - var->meta_range[0])*scale;
        }else{
            lock_bounds = (button == MouseButtonLeft);
            MouseMotion(view,x,y,mouse_state);
        }
    }else{
        if(!lock_bounds)
        {
            const double val = a->Get();
            var->meta_range[0] = min(var->meta_range[0], val);
            var->meta_range[1] = max(var->meta_range[1], val);
        }
    }
}

void Slider::MouseMotion(View&, int x, int y, int mouse_state)
{
    if( var->meta_range[0] != var->meta_range[1] )
    {
        const double range = (var->meta_range[1] - var->meta_range[0]);
        const double frac = (double)(x - v.l)/(double)v.w;
        if( lock_bounds )
        {
            const double bfrac = max(0.0,min(1.0,frac));
            a->Set(bfrac * range + var->meta_range[0] );
        }else{
            a->Set(frac * range + var->meta_range[0]);
        }
    }
}


void Slider::ResizeChildren()
{
    raster[0] = v.l+2;
    raster[1] = v.b + (v.h-text_height)/2.0;
}

void Slider::Render()
{
    const double val = a->Get();

    if( var->meta_range[0] != var->meta_range[1] )
    {
        DrawShadowRect(v);
        glColor4fv(colour_fg);
        glRect(v);
        glColor4fv(colour_dn);
        const double norm_val = max(0.0,min(1.0,(val - var->meta_range[0]) / (var->meta_range[1] - var->meta_range[0])));
        glRect(Viewport(v.l,v.b,v.w*norm_val,v.h));
    }

    glColor4fv(colour_tx);
    glRasterPos2fv( raster );
    glutBitmapString(font,(unsigned char*)title.c_str());

    std::ostringstream oss;
    oss << setprecision(4) << val;
    string str = oss.str();
    const int l = glutBitmapLength(font,(unsigned char*)str.c_str()) + 2;
    glRasterPos2f( v.l + v.w - l, raster[1] );
    glutBitmapString(font,(unsigned char*)str.c_str());
}


}
