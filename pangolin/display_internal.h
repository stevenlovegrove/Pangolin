#ifndef PANGOLIN_DISPLAY_INTERNAL_H
#define PANGOLIN_DISPLAY_INTERNAL_H

#include "platform.h"
#include "display.h"

namespace pangolin
{

  struct PangolinGl
  {
    PangolinGl();

    // Base container for displays
    View base;
    std::map<std::string,View*> all_views;

    // Reserve one texture
    GLuint tex;

    // State relating to interactivity
    bool quit;
    bool had_input;
    bool has_resized;
    int mouse_state;
    View* activeDisplay;

  };

}

#endif // PANGOLIN_DISPLAY_INTERNAL_H

