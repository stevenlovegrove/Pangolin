/* This file is part of the Pangolin Project.
 * http://github.com/stevenlovegrove/Pangolin
 *
 * Copyright (c) 2018 Andrey Mnatsakanov
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

#include <pangolin/factory/factory_registry.h>
#include <pangolin/platform.h>
#include <pangolin/gl/glinclude.h>
#include <pangolin/display/display.h>
#include <pangolin/display/display_internal.h>
#include <pangolin/display/window.h>

#include <pangolin/display/device/EmscriptenWindow.h>

#include <mutex>
#include <stdexcept>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

namespace pangolin
{

extern __thread PangolinGl* context;

std::mutex window_mutex;

  EmscriptenWindow:: EmscriptenWindow(const std::string& title, int width, int height)
  {
    PangolinGl::windowed_size[0] = width;
    PangolinGl::windowed_size[1] = height;
    EmscriptenWebGLContextAttributes attr;
    emscripten_webgl_init_context_attributes(&attr);
    attr.alpha = attr.depth = attr.stencil = attr.antialias = attr.preserveDrawingBuffer = attr.preferLowPowerToHighPerformance = attr.failIfMajorPerformanceCaveat = 1;
    attr.enableExtensionsByDefault = 1;
    attr.premultipliedAlpha = 0;
    attr.majorVersion = 1;
    attr.minorVersion = 0;
    ctx = emscripten_webgl_create_context(0, &attr);
    if( ctx < 0 ) {
      throw std::runtime_error("Pangolin Emscripten: Failed to create window." );
    }
  }
  
  EmscriptenWindow::~ EmscriptenWindow()
  {
    emscripten_webgl_destroy_context(ctx);
  }

  void EmscriptenWindow::MakeCurrent()
  {
    emscripten_webgl_make_context_current(ctx);
    emscripten_set_canvas_element_size("#canvas", PangolinGl::windowed_size[0], PangolinGl::windowed_size[1]);
    pangolin::glEngine().prog_fixed.Bind();
    context = this;
  }

  void  EmscriptenWindow::ToggleFullscreen()
  {
  }
  
  void EmscriptenWindow::Move(int x, int y)
  {
  }

  void EmscriptenWindow::Resize(unsigned int w, unsigned int h)
  {
  }
  
  void EmscriptenWindow::ProcessEvents()
  {
  }

  void  EmscriptenWindow::SwapBuffers() {
  }

  std::unique_ptr<WindowInterface> CreateEmscriptenWindowAndBind(const std::string& window_title, const int w, const int h)
  {
    EmscriptenWindow* win = new EmscriptenWindow(window_title, w, h);
    return std::unique_ptr<WindowInterface>(win);
  }

  PANGOLIN_REGISTER_FACTORY(EmscriptenWindow)
  {
    struct EmscriptenWindowFactory : public FactoryInterface<WindowInterface> {
      std::unique_ptr<WindowInterface> Open(const Uri& uri) override {
        const std::string window_title = uri.Get<std::string>("window_title", "window");
        const int w = uri.Get<int>("w", 640);
        const int h = uri.Get<int>("h", 480);
        return std::unique_ptr<WindowInterface>(CreateEmscriptenWindowAndBind(window_title, w, h));
      }
    };

    auto factory = std::make_shared<EmscriptenWindowFactory>();
    FactoryRegistry<WindowInterface>::I().RegisterFactory(factory, 10, "emscriptenwindow");
  }

}
