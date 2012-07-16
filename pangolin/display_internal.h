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

#ifndef PANGOLIN_DISPLAY_INTERNAL_H
#define PANGOLIN_DISPLAY_INTERNAL_H

#include "platform.h"
#include "display.h"
#include <boost/ptr_container/ptr_unordered_map.hpp>

#ifdef HAVE_CVARS
#include <GLConsole/GLConsole.h>
#endif // HAVE_CVARS

namespace pangolin
{

  struct PangolinGl
  {
    PangolinGl();

    // Base container for displays
    View base;

    // Named views which are managed by pangolin (i.e. created / deleted by pangolin)
    boost::ptr_unordered_map<const std::string,View> named_managed_views;

    // Global keypress hooks
    std::map<int,boost::function<void(void)> > keypress_hooks;

    // Manage fullscreen (ToggleFullscreen is quite new)
    bool is_double_buffered;
    bool is_fullscreen;
    GLint windowed_size[2];

    // State relating to interactivity
    bool quit;
    int had_input;
    int has_resized;
    int mouse_state;
    View* activeDisplay;

#ifdef HAVE_CVARS
    GLConsole console;
#endif // HAVE_CVARS

  };

}

#endif // PANGOLIN_DISPLAY_INTERNAL_H

