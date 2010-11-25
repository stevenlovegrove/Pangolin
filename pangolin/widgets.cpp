#include "widgets.h"

#include <iostream>
#include "gl_internal.h"

using namespace std;

namespace pangolin
{

extern __thread PangolinGl* context;

const static int border = 1;
const static int tab_w = 15;
const static int tab_h = 20;
const static int tab_p = 5;
const static float colour_s1[4] = {0.0, 0.0, 0.0, 1.0};
const static float colour_s2[4] = {0.3, 0.3, 0.3, 1.0};
const static float colour_bb[4] = {0.0, 0.0, 0.0, 0.1};
const static float colour_bg[4] = {1.0, 1.0, 1.0, 0.6};
const static float colour_fg[4] = {1.0, 1.0 ,1.0, 0.8};
const static float colour_tx[4] = {0.0, 0.0, 0.0, 1.0};
const static float colour_hl[4] = {0.9, 0.9, 0.9, 1.0};
const static float colour_dn[4] = {1.0, 0.5 ,0.5, 1.0};
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

void DrawShadowRect(Viewport& v, bool pushed)
{
  glColor4fv(pushed ? colour_s1 : colour_s2);
  glBegin(GL_LINE_STRIP);
    glVertex2f(v.r(),v.b);
    glVertex2f(v.l,v.b);
    glVertex2f(v.l,v.t());
  glEnd();

  glColor3fv(pushed ? colour_s2 : colour_s1);
  glBegin(GL_LINE_STRIP);
    glVertex2f(v.r(),v.b);
    glVertex2f(v.r(),v.t());
    glVertex2f(v.l,v.t());
  glEnd();
}

Panal::Panal()
{
  handler = &StaticHandler;
}

void Panal::Render()
{
  glColor4fv(colour_bb);
  glRect(v);
  glColor4fv(colour_bg);
  glRect(v);
  glRect(v,border);

  RenderChildren();
}

Button::Button(string title, _tvar<bool>& tv)
  : /*TVar<bool>(tv),*/ title(title)
{
  top = 1.0; bottom = -20;
  left = 0; right = 1.0;
  hlock = LockLeft;
  vlock = LockBottom;
  handler = this;
  text_width = glutBitmapLength(font,(unsigned char*)title.c_str());
}

void Button::Mouse(View&, int button, int state, int x, int y)
{
  *val = !*val;
}

void Button::Render()
{
  DrawShadowRect(v, *val);
  glColor4fv(*val ? colour_dn : colour_fg );
  glRect(vinside);
  glColor4fv(colour_tx);
  glRasterPos2fv(raster);
  glutBitmapString(font,(unsigned char*)title.c_str());
}

void Button::ResizeChildren()
{
  raster[0] = v.l + (v.w-text_width)/2.0;
  raster[1] = v.b + (v.h-text_height)/2.0;
  vinside = v.Inset(border);
}

Checkbox::Checkbox(std::string title, _tvar<bool>& tv)
  :/*TVar<bool>(tv),*/ title(title)
{
  top = 1.0; bottom = -20;
  left = 0; right = 1.0;
  hlock = LockLeft;
  vlock = LockBottom;
  handler = this;
}

void Checkbox::Mouse(View&, int button, int state, int x, int y)
{
  *val = !*val;
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
  DrawShadowRect(vcb, *val);
  if( *val )
  {
    glColor4fv(colour_dn);
    glRect(vcb);
  }
  glColor4fv(colour_tx);
  glRasterPos2fv( raster );
  glutBitmapString(font,(unsigned char*)title.c_str());
}


}
