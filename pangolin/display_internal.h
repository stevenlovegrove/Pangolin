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

    // Manage fullscreen (ToggleFullscreen is quite new)
    bool is_fullscreen;
    GLint windowed_size[2];

    // State relating to interactivity
    bool quit;
    bool had_input;
    bool has_resized;
    int mouse_state;
    View* activeDisplay;

  };

}

#endif // PANGOLIN_DISPLAY_INTERNAL_H

