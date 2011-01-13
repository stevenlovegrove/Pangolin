#ifndef PANGOLIN_WIDGETS_H
#define PANGOLIN_WIDGETS_H

#include "platform.h"
#include "display.h"
#include "vars.h"

namespace pangolin
{

View& CreatePanel(const std::string& name);

struct Panel : public View
{
  Panel();
  Panel(const std::string& auto_register_var_prefix);
  void Render();  
  void ResizeChildren();
  static void AddVariable(void* data, const std::string& name, _Var& var);
  Viewport vinside;
};

struct Button : public View, Handler, Var<bool>
{
  Button(std::string title, _Var& tv);
  void Mouse(View&, int button, int state, int x, int y);
  void Render();
  std::string title;

  //Cache params on resize
  void ResizeChildren();
  int text_width;
  GLfloat raster[2];
  Viewport vinside;
  bool down;
};

struct Checkbox : public View, Handler, Var<bool>
{
  Checkbox(std::string title, _Var& tv);
  void Mouse(View&, int button, int state, int x, int y);
  void Render();
  std::string title;

  //Cache params on resize
  void ResizeChildren();
  int text_width;
  GLfloat raster[2];
  Viewport vcb;
};

struct Slider : public View, Handler, Var<double>
{
  Slider(std::string title, _Var& tv);
  void Mouse(View&, int button, int state, int x, int y);
  void MouseMotion(View&, int x, int y);
  void Render();
  std::string title;

  //Cache params on resize
  void ResizeChildren();
  int text_width;
  GLfloat raster[2];
  bool lock_bounds;
};


}
#endif // PANGOLIN_WIDGETS_H
