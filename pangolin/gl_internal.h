#ifndef PANGOLIN_GL_INTERNAL_H
#define PANGOLIN_GL_INTERNAL_H

#include "platform.h"
#include "gl.h"

namespace pangolin
{

  struct PangolinGl
  {
    PangolinGl();

    // Base container for displays
    View base;
    std::map<std::string,View*> all_views;

//    // State relating to
//    View* currentDisplay;

    // State relating to interactivity
    bool quit;
    bool had_input;
    bool has_resized;
    int mouse_state;
    View* activeDisplay;

  };

}

#endif // PANGOLIN_GL_INTERNAL_H

