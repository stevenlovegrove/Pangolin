#ifndef PANGOLIN_GL_INTERNAL_H
#define PANGOLIN_GL_INTERNAL_H

#include "platform.h"
#include "gl.h"

namespace pangolin
{

  struct PangolinGl
  {
    PangolinGl();

    bool quit;
    DisplayContainer base;
  };

}

#endif // PANGOLIN_GL_INTERNAL_H

