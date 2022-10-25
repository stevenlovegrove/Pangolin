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

#pragma once

#include <pangolin/platform.h>
#include <pangolin/windowing/window.h>

#include <pangolin/display/view.h>
#include <pangolin/display/user_app.h>
#include <functional>
#include <memory>

#include <map>
#include <queue>

namespace pangolin
{

// Forward Declarations
class ConsoleView;
class GlFont;

typedef std::map<const std::string,View*> ViewMap;
typedef std::map<int,std::function<void(int)> > KeyhookMap;

struct PANGOLIN_EXPORT PangolinGl
{
    PangolinGl();
    ~PangolinGl();

    void Run();

    void MakeCurrent();
    void FinishFrame();

    void RenderViews();
    void PostRender();

    void SetOnRender(std::function<void()> on_render);

    // Callback for render loop
    std::function<void()> on_render;

    // Base container for displays
    View base;
    
    // Named views which are managed by pangolin (i.e. created / deleted by pangolin)
    ViewMap named_managed_views;

    // Optional user app
    UserApp* user_app;
    
    // Global keypress hooks
    KeyhookMap keypress_hooks;
    
    // State relating to interactivity
    bool quit;
    int mouse_state;
    View* activeDisplay;
    
    std::queue<std::pair<std::string,Viewport> > screen_capture;
    
    std::shared_ptr<WindowInterface> window;
    std::shared_ptr<GlFont> font;

    std::unique_ptr<ConsoleView> console_view;
};

PangolinGl* GetCurrentContext();
void SetCurrentContext(PangolinGl* context);
void RegisterNewContext(const std::string& name, std::shared_ptr<PangolinGl> newcontext);
void DeleteContext(const std::string& name);
PangolinGl *FindContext(const std::string& name);
void SetCurrentContext(PangolinGl* newcontext);

}

