#ifndef PANGOLIN_WIDGETS_H
#define PANGOLIN_WIDGETS_H

#include "platform.h"
#include "gl.h"
#include "vars.h"

namespace pangolin
{

View& CreatePanal(const std::string& name);

struct Panal : public View
{
  Panal();
  Panal(const std::string& auto_register_var_prefix);
  void Render();  
  static void AddVariable(void* data, const std::string& name, _Var& var);
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
};


}
#endif // PANGOLIN_WIDGETS_H
