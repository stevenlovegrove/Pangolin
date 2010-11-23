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
    Display base;

    // State relating to interactivity
    bool quit;
    int mouse_state;
    Display* activeDisplay;
  };

}

#endif // PANGOLIN_GL_INTERNAL_H

