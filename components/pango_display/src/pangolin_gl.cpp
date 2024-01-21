/* This file is part of the Pangolin Project.
 * http://github.com/stevenlovegrove/Pangolin
 *
 * Copyright (c) 2011 Steven Lovegrove, Richard Newcombe
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

#include <pangolin/display/pangolin_gl.h>
#include <pangolin/display/display.h>
#include <pangolin/console/ConsoleView.h>

namespace pangolin
{

PangolinGl::PangolinGl()
    : user_app(0), quit(false), mouse_state(0),activeDisplay(0)
{
}
PangolinGl::~PangolinGl()
{
    // Free displays owned by named_managed_views
    for(ViewMap::iterator iv = named_managed_views.begin(); iv != named_managed_views.end(); ++iv) {
        delete iv->second;
    }
    named_managed_views.clear();
}

void PangolinGl::MakeCurrent()
{
    SetCurrentContext(this);
    if(window) {
        window->MakeCurrent();
    }
}

void PangolinGl::RenderViews()
{
    Viewport::DisableScissor();
    base.Render();
}

void PangolinGl::FinishFrame()
{
    RenderViews();

    while(screen_capture.size()) {
        std::pair<std::string,Viewport> fv = screen_capture.front();
        screen_capture.pop();
        SaveWindowNow(fv.first, fv.second);
    }

    if(window) {
        window->SwapBuffers();
        window->ProcessEvents();
    }

    Viewport::DisableScissor();
}

void PangolinGl::SetOnRender(std::function<void ()> on_render) {
    this->on_render = on_render;
}

void PangolinGl::Run() {
    while (!quit) {
        if (on_render) {
            on_render();
        }
        FinishFrame();
    }
}

}
