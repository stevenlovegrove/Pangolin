/* This file is part of the Pangolin Project.
 * http://github.com/stevenlovegrove/Pangolin
 *
 * Copyright (c) 2011-2018 Steven Lovegrove, Richard Newcombe, Andrey Mnatsakanov
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


#include <pangolin/display/display.h>
#include <pangolin/factory/factory_registry.h>
#include <pangolin/window_frameworks.h>
#include <pangolin/platform.h>
#include <pangolin/gl/glinclude.h>
#include <pangolin/display/display.h>
#include <pangolin/display/display_internal.h>

#ifdef BUILD_PANGOLIN_VARS
  #include <pangolin/var/var.h>
#endif

#include <memory>
#include <sstream>

namespace pangolin
{

  extern __thread PangolinGl* context;

  bool one_time_window_frameworks_init = false;

  WindowInterface& CreateWindowAndBind(std::string window_title, int w, int h, const Params& params){

    if(!one_time_window_frameworks_init) {
      one_time_window_frameworks_init = LoadBuiltInWindowFrameworks();
    }

    std::stringstream s;
#if defined(_LINUX_)
    const std::string display_name = params.Get(PARAM_DISPLAYNAME, std::string());
    const bool double_buffered = params.Get(PARAM_DOUBLEBUFFER, true);
    const int  sample_buffers  = params.Get(PARAM_SAMPLE_BUFFERS, 1);
    const int  samples         = params.Get(PARAM_SAMPLES, 1);
    
    s << "x11window://["
      << "window_title=" << window_title << ","
      << "w=" << w << ","
      << "h=" << h << ","
      << "display_name=" << display_name << ","
      << "double_buffered=" << double_buffered << ","
      << "sample_buffers=" << sample_buffers << ","
      << "samples=" << samples 
      << "]";
    
#elif defined(_WIN_)    
    s << "x11window://["
      << "window_title=" << window_title << ","
      << "w=" << w << ","
      << "h=" << h 
      << "]";
#elif defined(_OSX_)
    const bool is_highres = params.Get(PARAM_HIGHRES, true);

    s << "x11window://["
      << "window_title=" << window_title << ","
      << "w=" << w << ","
      << "h=" << h << ","
      << "is_highres=" << is_highres
      << "]";
#else
#error "not detected any supported system"
#endif
    Uri uri = ParseUri(s.str());    
    std::unique_ptr<WindowInterface> window = FactoryRegistry<WindowInterface>::I().Open(uri);
    if(!window) {
      throw WindowExceptionNoKnownHandler(uri.scheme);
    }

    AddNewContext(window_title, std::shared_ptr<PangolinGl>(dynamic_cast<PangolinGl*>(window.release())) );
#if defined(_LINUX_)
    BindToContext(window_title);
    glewInit();
    context->ProcessEvents();
#elif defined(_OSX_)
    context->is_high_res = is_highres;
#elif defined(_WIN_)
    BindToContext(window_title);
    win->ProcessEvents();

    // Hack to make sure the window receives a
    while(!win->windowed_size[0]) {
      w -= 1; h -=1;
      win->Resize(w,h);
      win->ProcessEvents();
    }
    glewInit();
#else
#error "not detected any supported system"
#endif
    
    return *context;
  }
}
