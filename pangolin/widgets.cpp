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

#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
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
static int cb_height = text_height * 1.6;

boost::mutex display_mutex;

static bool guiVarHasChanged = true;

bool GuiVarHasChanged()
{
    return pangolin::Pushed(guiVarHasChanged);
}

template<typename T>
void GuiVarChanged( Var<T>& var)
{
  guiVarHasChanged = true;
  var.var->meta_gui_changed = true;

  BOOST_FOREACH(GuiVarChangedCallback& gvc, gui_var_changed_callbacks)
      if( boost::starts_with(var.var->meta_full_name,gvc.filter) )
      gvc.fn(gvc.data,var.var->meta_full_name,*var.var);
}

void glRect(Viewport v)
{
  glRecti(v.l,v.b,v.r(),v.t());
}

void glRect(Viewport v, int inset)
{
  glRecti(v.l+inset,v.b+inset,v.r()-inset,v.t()-inset);
}

void DrawShadowRect(Viewport& v)
{
  glColor4fv(colour_s2);
  glBegin(GL_LINE_STRIP);
  glVertex2i(v.l,v.b);
  glVertex2i(v.l,v.t());
  glVertex2i(v.r(),v.t());
  glVertex2i(v.r(),v.b);
  glVertex2i(v.l,v.b);
  glEnd();
}

void DrawShadowRect(Viewport& v, bool pushed)
{
  glColor4fv(pushed ? colour_s1 : colour_s2);
  glBegin(GL_LINE_STRIP);
  glVertex2i(v.l,v.b);
  glVertex2i(v.l,v.t());
  glVertex2i(v.r(),v.t());
  glEnd();

  glColor3fv(pushed ? colour_s2 : colour_s1);
  glBegin(GL_LINE_STRIP);
  glVertex2i(v.r(),v.t());
  glVertex2i(v.r(),v.b);
  glVertex2i(v.l,v.b);
  glEnd();
}

Panel::Panel()
  : context_views(context->named_managed_views)
{
  handler = &StaticHandlerScroll;
  layout = LayoutVertical;
}

Panel::Panel(const std::string& auto_register_var_prefix)
  : context_views(context->named_managed_views)
{
  handler = &StaticHandlerScroll;
  layout = LayoutVertical;
  RegisterNewVarCallback(&Panel::AddVariable,(void*)this,auto_register_var_prefix);

  // TODO: Work out how this might work
  //    ProcessHistoricCallbacks(&Panel::AddVariable,(void*)this,auto_register_var_prefix);
}

void Panel::AddVariable(void* data, const std::string& name, _Var& var, const char* reg_type_name, bool brand_new )
{
  Panel* thisptr = (Panel*)data;

  const string& title = var.meta_friendly;

  display_mutex.lock();

  boost::ptr_unordered_map<const std::string,View>::iterator pnl =
    thisptr->context_views.find(name);

  // Only add if a widget by the same name doesn't
  // already exist
  if( pnl == thisptr->context_views.end() )
  {
    if( reg_type_name == typeid(bool).name() )
    {
      View* nv = var.meta_flags ? (View*)new Checkbox(title,var) : (View*)new Button(title,var);
      //thisptr->context_views[name] = nv;
      thisptr->context_views.insert(name,nv);
      thisptr->views.push_back(nv);
      thisptr->ResizeChildren();
    }else if( reg_type_name == typeid(double).name() || reg_type_name == typeid(float).name() || reg_type_name == typeid(int).name() || reg_type_name == typeid(unsigned int).name() )
    {
      View* nv = new Slider(title,var);
      //thisptr->context_views[name] = nv;
      thisptr->context_views.insert(name,nv);
      thisptr->views.push_back( nv );
      thisptr->ResizeChildren();
    }else{
      View* nv = new TextInput(title,var);
      //thisptr->context_views[name] = nv;
      thisptr->context_views.insert(name,nv);
      thisptr->views.push_back( nv );
      thisptr->ResizeChildren();
    }
  }

  display_mutex.unlock();
}

void Panel::Render()
{
  glPushAttrib(GL_CURRENT_BIT | GL_ENABLE_BIT | GL_SCISSOR_BIT | GL_VIEWPORT_BIT);

  OpenGlRenderState::ApplyWindowCoords();
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_SCISSOR_TEST);
  glDisable(GL_LINE_SMOOTH);
  glLineWidth(1.0);

  glColor4fv(colour_s2);
  glRect(v);
  glColor4fv(colour_bg);
  glRect(v,1);

  RenderChildren();

  glPopAttrib();
}

void Panel::ResizeChildren()
{
  View::ResizeChildren();
}


View& CreatePanel(const std::string& name)
{
  Panel * p = new Panel(name);
  bool inserted = context->named_managed_views.insert(name, p).second;
  if(!inserted) throw exception();
  context->base.views.push_back(p);
  return *p;
}

Button::Button(string title, _Var& tv)
  : Widget<bool>(title,tv), down(false)
{
  top = 1.0; bottom = Attach::Pix(-20);
  left = 0.0; right = 1.0;
  hlock = LockLeft;
  vlock = LockBottom;
  text_width = glutBitmapLength(font,(unsigned char*)title.c_str());
}

void Button::Mouse(View&, MouseButton button, int x, int y, bool pressed, int mouse_state)
{
    if(button == MouseButtonLeft )
    {
      down = pressed;
      if( !pressed ) {
        a->Set(!a->Get());
        GuiVarChanged(*this);
      }
    }
}

void Button::Render()
{
  glColor4fv(colour_fg );
  glRect(v);
  glColor4fv(colour_tx);
  glRasterPos2f(raster[0],raster[1]-down);
  glutBitmapString(font,(unsigned char*)title.c_str());
  DrawShadowRect(v, down);
}

void Button::ResizeChildren()
{
  raster[0] = v.l + (v.w-text_width)/2.0;
  raster[1] = v.b + (v.h-text_height)/2.0;
  vinside = v.Inset(border);
}

Checkbox::Checkbox(std::string title, _Var& tv)
    : Widget<bool>(title,tv)
{
  top = 1.0; bottom = Attach::Pix(-20);
  left = 0.0; right = 1.0;
  hlock = LockLeft;
  vlock = LockBottom;
  handler = this;
}

void Checkbox::Mouse(View&, MouseButton button, int x, int y, bool pressed, int mouse_state)
{
  if( button == MouseButtonLeft && pressed ) {
    a->Set(!a->Get());
    GuiVarChanged(*this);
  }
}

void Checkbox::ResizeChildren()
{
  raster[0] = v.l + cb_height + 4;
  raster[1] = v.b + (v.h-text_height)/2.0;
  const int h = v.h;
  const int t = (h-cb_height) / 2.0;
  vcb = Viewport(v.l,v.b+t,cb_height,cb_height);
}

void Checkbox::Render()
{
  const bool val = a->Get();

  if( val )
  {
    glColor4fv(colour_dn);
    glRect(vcb);
  }
  glColor4fv(colour_tx);
  glRasterPos2fv( raster );
  glutBitmapString(font,(unsigned char*)title.c_str());
  DrawShadowRect(vcb, val);
}


Slider::Slider(std::string title, _Var& tv)
  : Widget<double>(title+":", tv), lock_bounds(true)
{
  top = 1.0; bottom = Attach::Pix(-20);
  left = 0.0; right = 1.0;
  hlock = LockLeft;
  vlock = LockBottom;
  handler = this;
  logscale = (int)tv.logscale;
}

void Slider::Keyboard(View&, unsigned char key, int x, int y, bool pressed){

  if( pressed && var->meta_range[0] < var->meta_range[1] )
  {
    double val = !logscale ? a->Get() : log(a->Get());

    if(key=='-'){
      if (logscale)
        a->Set( exp(max(var->meta_range[0],min(var->meta_range[1],val - var->meta_increment) ) ) );
      else
        a->Set( max(var->meta_range[0],min(var->meta_range[1],val - var->meta_increment) ) );
    }else if(key == '='){
      if (logscale)
        a->Set( exp(max(var->meta_range[0],min(var->meta_range[1],val + var->meta_increment) ) ) );
      else
        a->Set( max(var->meta_range[0],min(var->meta_range[1],val + var->meta_increment) ) );
    }else if(key == 'r'){
      Reset();
    }else{
      return;
    }
    GuiVarChanged(*this);
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
      double val = frac * (var->meta_range[1] - var->meta_range[0]) + var->meta_range[0];

      if (logscale)
      {
        if (val<=0)
          val = std::numeric_limits<double>::min();
        else
          val = log(val);
      }

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
      double val = !logscale ? a->Get() : log(a->Get());

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
    double val;

    if( lock_bounds )
    {
      const double bfrac = max(0.0,min(1.0,frac));
      val = bfrac * range + var->meta_range[0] ;
    }else{
      val = frac * range + var->meta_range[0];
    }

    if (logscale) val = exp(val);

     a->Set(val);
     GuiVarChanged(*this);
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
    double rval = val;
    if (logscale)
    {
      rval = log(val);
    }
    glColor4fv(colour_fg);
    glRect(v);
    glColor4fv(colour_dn);
    const double norm_val = max(0.0,min(1.0,(rval - var->meta_range[0]) / (var->meta_range[1] - var->meta_range[0])));
    glRect(Viewport(v.l,v.b,v.w*norm_val,v.h));
    DrawShadowRect(v);
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


TextInput::TextInput(std::string title, _Var& tv)
  : Widget<std::string>(title+":", tv), do_edit(false)
{
  top = 1.0; bottom = Attach::Pix(-20);
  left = 0.0; right = 1.0;
  hlock = LockLeft;
  vlock = LockBottom;
  handler = this;
  sel[0] = -1;
  sel[1] = -1;
}

void TextInput::Keyboard(View&, unsigned char key, int x, int y, bool pressed)
{
  if(pressed)
  {
    const bool selection = sel[1] > sel[0] && sel[0] >= 0;

    if(key == 13)
    {
      a->Set(edit);
      GuiVarChanged(*this);

      do_edit = false;
      sel[0] = sel[1] = -1;
    }else if(key == 8) {
      // backspace
      if(selection)
      {
        edit = edit.substr(0,sel[0]) + edit.substr(sel[1],edit.length()-sel[1]);
        sel[1] = sel[0];
      }else{
        if(sel[0] >0)
        {
          edit = edit.substr(0,sel[0]-1) + edit.substr(sel[0],edit.length()-sel[0]);
          sel[0]--;
          sel[1]--;
        }
      }
    }else if(key == 127){
      // delete
      if(selection)
      {
        edit = edit.substr(0,sel[0]) + edit.substr(sel[1],edit.length()-sel[1]);
        sel[1] = sel[0];
      }else{
        if(sel[0] < (int)edit.length())
        {
          edit = edit.substr(0,sel[0]) + edit.substr(sel[0]+1,edit.length()-sel[0]+1);
        }
      }
    }else if(key == 230){
      // right
      sel[0] = min((int)edit.length(),sel[0]+1);
      sel[1] = sel[0];
    }else if(key == 228){
      // left
      sel[0] = max(0,sel[0]-1);
      sel[1] = sel[0];
    }else if(key == 234){
      // home
      sel[0] = sel[1] = 0;
    }else if(key == 235){
      // end
      sel[0] = sel[1] = edit.length();
    }else{
      //            cout << (int)key << endl;
      edit = edit.substr(0,sel[0]).append(1,key) + edit.substr(sel[1],edit.length()-sel[1]);
      sel[1] = sel[0];
      sel[0]++;
      sel[1]++;
    }
  }
}

void TextInput::Mouse(View& view, MouseButton button, int x, int y, bool pressed, int mouse_state)
{
    if(button != MouseWheelUp && button != MouseWheelDown )
    {

      if(do_edit)
      {
        const int sl = glutBitmapLength(font,(unsigned char*)edit.c_str()) + 2;
        const int rl = v.l + v.w - sl;
        int ep = edit.length();

        if( x < rl )
        {
          ep = 0;
        }else{
          for( unsigned i=0; i<edit.length(); ++i )
          {
            const int tl = rl + glutBitmapLength(font,(unsigned char*)edit.substr(0,i).c_str());
            if(x < tl+2)
            {
              ep = i;
              break;
            }
          }
        }
        if(pressed)
        {
          sel[0] = sel[1] = ep;
        }else{
          sel[1] = ep;
        }

        if(sel[0] > sel[1])
          std::swap(sel[0],sel[1]);
      }else{
        do_edit = !pressed;
        sel[0] = 0;
        sel[1] = edit.length();
      }
    }
}

void TextInput::MouseMotion(View&, int x, int y, int mouse_state)
{
  if(do_edit)
  {
    const int sl = glutBitmapLength(font,(unsigned char*)edit.c_str()) + 2;
    const int rl = v.l + v.w - sl;
    int ep = edit.length();

    if( x < rl )
    {
      ep = 0;
    }else{
      for( unsigned i=0; i<edit.length(); ++i )
      {
        const int tl = rl + glutBitmapLength(font,(unsigned char*)edit.substr(0,i).c_str());
        if(x < tl+2)
        {
          ep = i;
          break;
        }
      }
    }

    sel[1] = ep;
  }
}


void TextInput::ResizeChildren()
{
  raster[0] = v.l+2;
  raster[1] = v.b + (v.h-text_height)/2.0;
}

void TextInput::Render()
{
  if(!do_edit) edit = a->Get();

  glColor4fv(colour_fg);
  glRect(v);

  const int sl = glutBitmapLength(font,(unsigned char*)edit.c_str()) + 2;
  const int rl = v.l + v.w - sl;

  if( do_edit && sel[0] >= 0)
  {
    const int tl = rl + glutBitmapLength(font,(unsigned char*)edit.substr(0,sel[0]).c_str());
    const int tr = rl + glutBitmapLength(font,(unsigned char*)edit.substr(0,sel[1]).c_str());
    glColor4fv(colour_dn);
    glRect(Viewport(tl,v.b,tr-tl,v.h));
  }

  glColor4fv(colour_tx);
  glRasterPos2fv( raster );
  glutBitmapString(font,(unsigned char*)title.c_str());

  glRasterPos2f( rl, raster[1] );
  glutBitmapString(font,(unsigned char*)edit.c_str());
  DrawShadowRect(v);
}

}
