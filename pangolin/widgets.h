#ifndef PANGOLIN_WIDGETS_H
#define PANGOLIN_WIDGETS_H

#include "platform.h"
#include "gl.h"
#include "vars.h"

namespace pangolin
{



struct Panal : public View
{
  Panal();
  void Render();
};

struct Button : public View, Handler//, TVar<bool>
{
  Button(std::string title, _tvar<bool>& tv);
  void Mouse(View&, int button, int state, int x, int y);
  void Render();
  std::string title;

  //Cache params on resize
  void ResizeChildren();
  int text_width;
  GLfloat raster[2];
  Viewport vinside;
  bool* val;
};

struct Checkbox : public View, Handler//, TVar<bool>
{
  Checkbox(std::string title, _tvar<bool>& tv);
  void Mouse(View&, int button, int state, int x, int y);
  void Render();
  std::string title;

  //Cache params on resize
  void ResizeChildren();
  int text_width;
  GLfloat raster[2];
  Viewport vcb;
  bool* val;

};


}
#endif // PANGOLIN_WIDGETS_H
