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

#include <pangolin/platform.h>


#include <iostream>
#include <sstream>
#include <string>
#include <map>
#include <mutex>
#include <cstdlib>
#include <memory>

#include <pangolin/gl/gl.h>
#include <pangolin/gl/gldraw.h>
#include <pangolin/factory/factory_registry.h>
#include <pangolin/display/display.h>
#include <pangolin/display/process.h>
#include <pangolin/console/ConsoleView.h>
#include <pangolin/utils/simple_math.h>
#include <pangolin/utils/timer.h>
#include <pangolin/utils/type_convert.h>
#include <pangolin/image/image_io.h>
#include <pangolin/var/var.h>

#include "pangolin_gl.h"

extern const unsigned char AnonymousPro_ttf[];

namespace pangolin
{

typedef std::map<std::string,std::shared_ptr<PangolinGl> > ContextMap;

// Map of active contexts
ContextMap contexts;
std::recursive_mutex contexts_mutex;

// Context active for current thread
__thread PangolinGl* context = 0;

void SetCurrentContext(PangolinGl* newcontext) {
    context = newcontext;
}

PangolinGl* GetCurrentContext()
{
    return context;
}

PangolinGl *FindContext(const std::string& name)
{
    contexts_mutex.lock();
    ContextMap::iterator ic = contexts.find(name);
    PangolinGl* context = (ic == contexts.end()) ? 0 : ic->second.get();
    contexts_mutex.unlock();
    return context;
}

WindowInterface& CreateWindowAndBind(std::string window_title, int w, int h, const Params& params)
{
    std::unique_lock<std::recursive_mutex> l(contexts_mutex);

    pangolin::Uri win_uri;

    if(const char* extra_params = std::getenv("PANGOLIN_WINDOW_URI"))
    {
        // Take any defaults from the environment
        win_uri = pangolin::ParseUri(extra_params);
    }else{
        // Otherwise revert to 'default' scheme.
        win_uri.scheme = "default";
    }

    // Override with anything the program specified
    for(const auto& param : params.params) {
        if(param.first != "scheme") win_uri.params.push_back(param);
    }

    // Special params that shouldn't get passed to window factory
    win_uri.scheme = win_uri.Get("scheme", win_uri.scheme);
    const std::string default_font = win_uri.Get<std::string>("default_font","");
    const int default_font_size = win_uri.Get("default_font_size", 18);
    win_uri.Remove("scheme");
    win_uri.Remove("default_font");
    win_uri.Remove("default_font_size");

    // Additional arguments we will send to factory
    win_uri.Set("w", w);
    win_uri.Set("h", h);
    win_uri.Set("window_title", window_title);

    auto context = std::make_shared<PangolinGl>();
    context->window = ConstructWindow(win_uri);
    assert(context->window);

    RegisterNewContext(window_title, context);

    context->window->CloseSignal.connect( [](){
        pangolin::Quit();
    });
    context->window->ResizeSignal.connect( [](WindowResizeEvent event){
        process::Resize(event.width, event.height);
    });
    context->window->KeyboardSignal.connect( [](KeyboardEvent event) {
        process::Keyboard(event.key, event.x, event.y, event.pressed, event.key_modifiers);
    });
    context->window->MouseSignal.connect( [](MouseEvent event){
        process::Mouse(event.button, event.pressed, event.x, event.y, event.key_modifiers);
    });
    context->window->MouseMotionSignal.connect( [](MouseMotionEvent event){
        process::MouseMotion(event.x, event.y, event.key_modifiers);
    });
    context->window->PassiveMouseMotionSignal.connect( [](MouseMotionEvent event){
        process::PassiveMouseMotion(event.x, event.y, event.key_modifiers);
    });
    context->window->SpecialInputSignal.connect( [](SpecialInputEvent event){
        process::SpecialInput(event.inType, event.x, event.y, event.p[0], event.p[1], event.p[2], event.p[3], event.key_modifiers);
    });

    // If there is a special font request
    if( !default_font.empty() ) {
        const std::string font_filename = PathExpand(default_font);
        context->font = std::make_shared<GlFont>(font_filename, default_font_size);
    }else{
        context->font = std::make_shared<GlFont>(AnonymousPro_ttf, default_font_size);
    }

    context->MakeCurrent();
#ifdef HAVE_GLEW
    glewInit();
#endif

    // And finally process pending window events (such as resize) now that we've setup our callbacks.
    context->window->ProcessEvents();

    return *context->window;
}

// Assumption: unique lock is held on contexts_mutex for multi-threaded operation
void RegisterNewContext(const std::string& name, std::shared_ptr<PangolinGl> newcontext)
{
    // Set defaults
    newcontext->base.left = 0.0;
    newcontext->base.bottom = 0.0;
    newcontext->base.top = 1.0;
    newcontext->base.right = 1.0;
    newcontext->base.aspect = 0;
    newcontext->base.handler = &StaticHandler;

    // Create and add
    if( contexts.find(name) != contexts.end() ) {
        throw std::runtime_error("Context already exists.");
    }
    contexts[name] = newcontext;

    // Process the following as if this context is now current.
    PangolinGl *oldContext = context;
    context = newcontext.get();

    // Default key bindings can be overridden
    RegisterKeyPressCallback(PANGO_KEY_ESCAPE, Quit );
    RegisterKeyPressCallback('\t', [](){ShowFullscreen(TrueFalseToggle::Toggle);} );
    RegisterKeyPressCallback('`',  [](){ShowConsole(TrueFalseToggle::Toggle);} );

    context = oldContext;
}

WindowInterface* GetBoundWindow()
{
    return context->window.get();
}

void DestroyWindow(const std::string& name)
{
    contexts_mutex.lock();
    ContextMap::iterator ic = contexts.find(name);
    PangolinGl *context_to_destroy = (ic == contexts.end()) ? 0 : ic->second.get();
    if (context_to_destroy == context) {
        context = nullptr;
    }
    size_t erased = contexts.erase(name);
    if(erased == 0) {
        pango_print_warn("Context '%s' doesn't exist for deletion.\n", name.c_str());
    }
    contexts_mutex.unlock();
}

void BindToContext(std::string name)
{
    std::unique_lock<std::recursive_mutex> l(contexts_mutex);

    // N.B. context is modified prior to invoking MakeCurrent so that
    // state management callbacks (such as Resize()) can be correctly
    // processed.
    PangolinGl *context_to_bind = FindContext(name);
    if( !context_to_bind )
    {
        std::shared_ptr<PangolinGl> newcontext(new PangolinGl());
        RegisterNewContext(name, newcontext);
    }else{
        context_to_bind->MakeCurrent();
    }
}

void Quit()
{
    context->quit = true;
}

void QuitAll()
{
    for(const auto& nc : contexts) {
        nc.second->quit = true;
    }
}

bool ShouldQuit()
{
    return !context || context->quit;
}

void ShowFullscreen(TrueFalseToggle on_off)
{
    if(context && context->window)
        context->window->ShowFullscreen(on_off);
}


void FinishFrame()
{
    if(context) context->FinishFrame();
}

View& DisplayBase()
{
    return context->base;
}

View& CreateDisplay()
{
    int iguid = rand();
    std::stringstream ssguid;
    ssguid << iguid;
    return Display(ssguid.str());
}

void ShowConsole(TrueFalseToggle on_off)
{
    if( !context->console_view) {
        Uri interpreter_uri = ParseUri("python://");

        try {
            // Instantiate interpreter
            std::shared_ptr<InterpreterInterface> interpreter
                    = FactoryRegistry::I()->Construct<InterpreterInterface>(interpreter_uri);
            assert(interpreter);

            // Create console and let the pangolin context take ownership
            context->console_view = std::make_unique<ConsoleView>(interpreter);
            context->console_view->zorder = std::numeric_limits<int>::max();
            DisplayBase().AddDisplay(*context->console_view);
            context->console_view->SetFocus();
        }  catch (std::exception&) {
        }
    }else{
        context->console_view->Show(
            to_bool(on_off, context->console_view->IsShown())
        );

        if(context->console_view->IsShown()) {
            context->console_view->SetFocus();
        }
    }

}

View& Display(const std::string& name)
{
    // Get / Create View
    ViewMap::iterator vi = context->named_managed_views.find(name);
    if( vi != context->named_managed_views.end() )
    {
        return *(vi->second);
    }else{
        View * v = new View();
        context->named_managed_views[name] = v;
        v->handler = &StaticHandler;
        context->base.views.push_back(v);
        return *v;
    }
}

void RegisterKeyPressCallback(int key, std::function<void(int)> func)
{
    context->keypress_hooks[key] = func;
}

void RegisterKeyPressCallback(int key, std::function<void(void)> func)
{
    context->keypress_hooks[key] = [=](int){func();};
}

void SaveWindowOnRender(const std::string& filename, const Viewport& v)
{
    context->screen_capture.push(std::pair<std::string,Viewport>(filename, v) );
}

void SaveWindowNow(const std::string& filename_hint, const Viewport& v)
{
    const Viewport to_save = v.area() ? v.Intersect(DisplayBase().v) : DisplayBase().v;
    std::string filename = filename_hint;
    if(FileLowercaseExtention(filename) == "")  filename += ".png";

    try {
        glFlush();
        TypedImage buffer = ReadFramebuffer(to_save, "RGBA32");
        SaveImage(buffer, filename, false);
    }  catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
}

}
