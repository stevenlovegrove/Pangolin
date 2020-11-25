/* This file is part of the Pangolin Project.
 * http://github.com/stevenlovegrove/Pangolin
 *
 * Copyright (c) 2011-2018 Steven Lovegrove, Andrey Mnatsakanov
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

// Silence all the OSX GL deprecation messages.
#define GL_SILENCE_DEPRECATION

#include <pangolin/factory/factory_registry.h>
#include <pangolin/platform.h>
#include <pangolin/gl/glinclude.h>
#include <pangolin/windowing/OsxWindow.h>
#include <pangolin/windowing/PangolinNSGLView.h>
#include <pangolin/windowing/PangolinNSApplication.h>
#include <memory>

#if MAC_OS_X_VERSION_MAX_ALLOWED >= 101200
#  define NSFullScreenWindowMask      NSWindowStyleMaskFullScreen
#  define NSTitledWindowMask          NSWindowStyleMaskTitled
#  define NSMiniaturizableWindowMask  NSWindowStyleMaskMiniaturizable
#  define NSResizableWindowMask       NSWindowStyleMaskResizable
#  define NSClosableWindowMask        NSWindowStyleMaskClosable
#endif

// Hack to fix window focus issue
// http://www.miscdebris.net/blog/2010/03/30/solution-for-my-mac-os-x-gui-program-doesnt-get-focus-if-its-outside-an-application-bundle/
extern "C" { void CPSEnableForegroundOperation(ProcessSerialNumber* psn); }
inline void FixOsxFocus()
{
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    ProcessSerialNumber psn;
    GetCurrentProcess( &psn );
    CPSEnableForegroundOperation( &psn );
    SetFrontProcess( &psn );
#pragma clang diagnostic pop
}

namespace pangolin
{

std::unique_ptr<WindowInterface> CreateOsxWindowAndBind(std::string window_title, int w, int h, const bool is_highres)
{

    OsxWindow* win = new OsxWindow(window_title, w, h, is_highres);

    return std::unique_ptr<WindowInterface>(win);
}

OsxWindow::OsxWindow(
    const std::string& title, int width, int height, bool USE_RETINA
) {
    ///////////////////////////////////////////////////////////////////////
    // Make sure Application is initialised correctly.
    // This can be run repeatedly.

    [NSApplication sharedApplication];
    PangolinAppDelegate *delegate = [[PangolinAppDelegate alloc] init];

    [NSApp setDelegate:delegate];
    [NSApp setPresentationOptions:NSFullScreenWindowMask];

    [PangolinNSApplication run_pre];
    [PangolinNSApplication run_step];

    ///////////////////////////////////////////////////////////////////////
    // Create Window

    NSRect viewRect = NSMakeRect( 0.0, 0.0, width, height );

    _window = [[NSWindow alloc] initWithContentRect:viewRect styleMask:NSTitledWindowMask|NSMiniaturizableWindowMask|NSResizableWindowMask|NSClosableWindowMask backing:NSBackingStoreBuffered defer:YES];
    [_window setTitle:[NSString stringWithUTF8String:title.c_str()]];
    [_window setOpaque:YES];
    [_window makeKeyAndOrderFront:NSApp];
    [_window setCollectionBehavior: NSWindowCollectionBehaviorFullScreenPrimary];

    PangolinWindowDelegate *windelegate = [[PangolinWindowDelegate alloc] init];
    [_window setDelegate:windelegate];
    windelegate->osx_window = this;

    ///////////////////////////////////////////////////////////////////////
    // Create OpenGL View for Window

    NSOpenGLPixelFormatAttribute attrs[] =
    {
        NSOpenGLPFADoubleBuffer,
        NSOpenGLPFADepthSize, 32,
        NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersionLegacy,
        0
    };

    NSOpenGLPixelFormat *format = [[NSOpenGLPixelFormat alloc] initWithAttributes:attrs];
    view = [[PangolinNSGLView alloc] initWithFrame:_window.frame pixelFormat:format];
    view->osx_window = this;

    [format release];
#if MAC_OS_X_VERSION_MAX_ALLOWED >= 1070
    if( USE_RETINA && floor(NSAppKitVersionNumber) > NSAppKitVersionNumber10_6)
        [view setWantsBestResolutionOpenGLSurface:YES];
#endif /*MAC_OS_X_VERSION_MAX_ALLOWED*/

    [_window setContentView:view];

    glewInit();

    FixOsxFocus();
}

OsxWindow::~OsxWindow()
{
    // Not sure how to deallocate...
}


void OsxWindow::ShowFullscreen(const TrueFalseToggle on_off)
{
    const bool is_fullscreen = ([_window styleMask] & NSFullScreenWindowMask) == NSFullScreenWindowMask;
    if(should_toggle(on_off, is_fullscreen) ) {
        [_window toggleFullScreen:nil];
    }
}

void OsxWindow::Move(int x, int y)
{
    [_window setFrame:CGRectMake(x, y, [_window frame].size.width,
      [_window frame].size.height) display:NO];
}

void OsxWindow::Resize(unsigned int w, unsigned int h)
{
    const CGFloat title_height = _window.frame.size.height -
      [_window contentRectForFrameRect: _window.frame].size.height;

    [_window setFrame:CGRectMake([_window frame].origin.x,
      [_window frame].origin.y, w, h+title_height) display:NO];
}

void OsxWindow::MakeCurrent()
{
    [[view openGLContext] makeCurrentContext];
}

void OsxWindow::RemoveCurrent()
{
    [NSOpenGLContext clearCurrentContext];
}

void OsxWindow::SwapBuffers()
{
    [[view openGLContext] flushBuffer];
}

void OsxWindow::ProcessEvents()
{
    [PangolinNSApplication run_step];
}

PANGOLIN_REGISTER_FACTORY(OsxWindow)
{
  struct OsxWindowFactory : public TypedFactoryInterface<WindowInterface> {
    std::map<std::string,Precedence> Schemes() const override
    {
        return {{"cocoa",10}, {"default",10}};
    }
    const char* Description() const override
    {
        return "Use MacOS native window toolkit";
    }
    ParamSet Params() const override
    {
        return {{
            {"window_title","window","Title of application Window"},
            {"HIGHRES","true","Use 'retina' resolution"},
            {"w","640","Requested window width"},
            {"h","480","Requested window height"},
        }};
    }
    std::unique_ptr<WindowInterface> Open(const Uri& uri) override {

      const std::string window_title = uri.Get<std::string>("window_title", "window");
      const int w = uri.Get<int>("w", 640);
      const int h = uri.Get<int>("h", 480);
      const bool is_highres = uri.Get<bool>(PARAM_HIGHRES, true);
      return std::unique_ptr<WindowInterface>(CreateOsxWindowAndBind(window_title, w, h, is_highres));
    }
  };

  return FactoryRegistry::I()->RegisterFactory<WindowInterface>(std::make_shared<OsxWindowFactory>());
}

}
