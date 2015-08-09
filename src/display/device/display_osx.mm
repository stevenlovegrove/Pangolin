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

#include <pangolin/platform.h>
#include <pangolin/gl/glinclude.h>
#include <pangolin/display/display.h>
#include <pangolin/display/display_internal.h>

#import <Cocoa/Cocoa.h>

static float backing_scale = 1.0;
static std::string window_name = "";

namespace pangolin
{
extern __thread PangolinGl* context;
}

////////////////////////////////////////////////////////////////////
// PangolinNSGLView
////////////////////////////////////////////////////////////////////

@interface PangolinNSGLView : NSOpenGLView
{
}
@end

@implementation PangolinNSGLView

-(id)initWithFrame:(NSRect)frameRect pixelFormat:(NSOpenGLPixelFormat *)format
{
    self = [super initWithFrame:frameRect pixelFormat:format];
    return(self);
}

- (void)prepareOpenGL
{
    [super prepareOpenGL];
}

-(void)reshape
{
#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_7
    if ( [ _window respondsToSelector:@selector(backingScaleFactor) ] )
        backing_scale = [_window backingScaleFactor];
    else
#endif
        backing_scale = 1.0;

    pangolin::process::Resize(self.bounds.size.width * backing_scale, self.bounds.size.height * backing_scale);

    [[self openGLContext] update];
}

-(BOOL)acceptsFirstResponder
{
    return(YES);
}

-(BOOL)becomeFirstResponder
{
    return(YES);
}

-(BOOL)resignFirstResponder
{
    return(YES);
}

-(BOOL)isFlipped
{
    return(YES);
}

////////////////////////////////////////////////////////////////////
// Input maps
////////////////////////////////////////////////////////////////////

static int mapMouseButton(int osx_button )
{
    const int map[] = {0, 2, 1, 3, 4, 5, 6, 7, 8, 9, 10};
    return map[osx_button];
}

static int mapKeymap(int osx_key)
{
    if(osx_key == NSUpArrowFunctionKey)
        return pangolin::PANGO_SPECIAL + pangolin::PANGO_KEY_UP;
    else if(osx_key == NSDownArrowFunctionKey)
        return pangolin::PANGO_SPECIAL + pangolin::PANGO_KEY_DOWN;
    else if(osx_key == NSLeftArrowFunctionKey)
        return pangolin::PANGO_SPECIAL + pangolin::PANGO_KEY_LEFT;
    else if(osx_key == NSRightArrowFunctionKey)
        return pangolin::PANGO_SPECIAL + pangolin::PANGO_KEY_RIGHT;
    else if(osx_key == NSPageUpFunctionKey)
        return pangolin::PANGO_SPECIAL + pangolin::PANGO_KEY_PAGE_UP;
    else if(osx_key == NSPageDownFunctionKey)
        return pangolin::PANGO_SPECIAL + pangolin::PANGO_KEY_PAGE_DOWN;
    else if(osx_key == NSHomeFunctionKey)
        return pangolin::PANGO_SPECIAL + pangolin::PANGO_KEY_HOME;
    else if(osx_key == NSEndFunctionKey)
        return pangolin::PANGO_SPECIAL + pangolin::PANGO_KEY_END;
    else if(osx_key == NSInsertFunctionKey)
        return pangolin::PANGO_SPECIAL + pangolin::PANGO_KEY_INSERT;
    else if(osx_key == NSDeleteCharacter )
        return NSBackspaceCharacter;
    else if(osx_key == NSDeleteFunctionKey)
        return NSDeleteCharacter;
    else {
        return osx_key;
    }
}

-(NSPoint)_Location:(NSEvent *)theEvent
{
    NSPoint location = [self convertPoint: [theEvent locationInWindow] fromView: nil];
    location.x *= backing_scale;
    location.y *= backing_scale;
    return location;
}

////////////////////////////////////////////////////////////////////
// Keyboard
////////////////////////////////////////////////////////////////////

-(void)keyDown:(NSEvent *)theEvent
{
    const NSPoint location = [self _Location: theEvent];
    NSString *str = [theEvent characters];
    int len = (int)[str length];
    for(int i = 0; i < len; i++)
    {
        const int osx_key = [str characterAtIndex:i];
        pangolin::process::Keyboard(mapKeymap(osx_key), location.x, location.y);
    }
}

-(void)keyUp:(NSEvent *)theEvent
{
    const NSPoint location = [self _Location: theEvent];
    NSString *str = [theEvent characters];
    int len = (int)[str length];
    for(int i = 0; i < len; i++)
    {
        const int osx_key = [str characterAtIndex:i];
        pangolin::process::KeyboardUp(mapKeymap(osx_key), location.x, location.y);
    }
}

- (void)flagsChanged:(NSEvent *)event
{
//    NSLog(@"flagsChanged");
//    unsigned int flags = [event modifierFlags] & NSDeviceIndependentModifierFlagsMask;
//    flags & NSShiftKeyMask;
//    flags & NSCommandKeyMask;
}

////////////////////////////////////////////////////////////////////
// Mouse Input
////////////////////////////////////////////////////////////////////

-(void)mouseDownCommon:(NSEvent *)theEvent
{
    const int button = (int)[theEvent buttonNumber];
    const NSPoint location = [self _Location: theEvent];
    pangolin::process::Mouse(mapMouseButton(button), 0, location.x, location.y);
}

-(void)mouseUpCommon:(NSEvent *)theEvent
{
    const int button = (int)[theEvent buttonNumber];
    const NSPoint location = [self _Location: theEvent];
    pangolin::process::Mouse(mapMouseButton(button), 1, location.x, location.y);
}

- (void)mouseDraggedCommon: (NSEvent *)theEvent
{
    const NSPoint location = [self _Location: theEvent];
    pangolin::process::MouseMotion(location.x, location.y);
//    pangolin::process::SubpixMotion(location.x, location.y, 1.0, 0.0, 0.0, 0.0);
}

-(void)mouseDown:(NSEvent *)theEvent
{
    [self mouseDownCommon:theEvent];
}

-(void)mouseUp:(NSEvent *)theEvent
{
    [self mouseUpCommon:theEvent];
}

- (void)mouseDragged: (NSEvent *)theEvent
{
    [self mouseDraggedCommon:theEvent];
}

-(void)rightMouseDown:(NSEvent *)theEvent
{
    [self mouseDownCommon:theEvent];
}

-(void)rightMouseUp:(NSEvent *)theEvent
{
    [self mouseUpCommon:theEvent];
}

- (void)rightMouseDragged: (NSEvent *)theEvent
{
    [self mouseDraggedCommon:theEvent];
}

-(void)otherMouseDown:(NSEvent *)theEvent
{
    [self mouseDownCommon:theEvent];
}

-(void)otherMouseUp:(NSEvent *)theEvent
{
    [self mouseUpCommon:theEvent];
}

- (void)otherMouseDragged: (NSEvent *)theEvent
{
    [self mouseDraggedCommon:theEvent];
}

- (void)mouseMoved: (NSEvent *)theEvent
{
    const NSPoint location = [self _Location: theEvent];
//    pangolin::process::PassiveMouseMotion(location.x, location.y);
    pangolin::process::SubpixMotion(location.x, location.y, 0.0, 0.0, 0.0, 0.0);
}

- (void)scrollWheel:(NSEvent *)theEvent
{
    const NSPoint location = [self _Location: theEvent];

    float dx, dy;
    if([theEvent respondsToSelector:@selector(scrollingDeltaX)]) {
       dx = theEvent.scrollingDeltaX; dy = theEvent.scrollingDeltaY;
    } else {
       dx = theEvent.deltaX; dy = theEvent.deltaY;
    }

    if(dx != 0.0f || dy != 0.0f) {
        pangolin::process::SpecialInput(
            pangolin::InputSpecialScroll,
            location.x, pangolin::context->base.v.h - location.y,
            dx, dy,
            0, 0
        );
    }
}

- (void)magnifyWithEvent: (NSEvent *)theEvent
{
    const NSPoint location = [self _Location: theEvent];
    const float dm = theEvent.magnification;
    if(dm != 0.0f) {
        pangolin::process::SpecialInput(
                pangolin::InputSpecialZoom,
                location.x, pangolin::context->base.v.h - location.y,
                dm, 0.0f, 0.0f, 0.0f
        );
    }
}

- (void)rotateWithEvent: (NSEvent *)theEvent
{
    const NSPoint location = [self _Location: theEvent];
    const float dr = theEvent.rotation;
    if(dr != 0.0f) {
        pangolin::process::SpecialInput(
                pangolin::InputSpecialRotate,
                location.x, pangolin::context->base.v.h - location.y,
                dr, 0.0f, 0.0f, 0.0f
        );
    }
}

- (void)mouseEntered: (NSEvent *)theEvent
{
}

- (void)mouseExited: (NSEvent *)theEvent
{
}

-(void)dealloc
{
    [super dealloc];
}

@end

static PangolinNSGLView *view = 0;

////////////////////////////////////////////////////////////////////
// PangolinNSApplication
////////////////////////////////////////////////////////////////////

@interface PangolinNSApplication : NSApplication
{
}

- (void)run_pre;
- (void)run_step;
//- (void)terminate:(id)sender;

@end

@implementation PangolinNSApplication

- (void)run_pre
{
    [[NSNotificationCenter defaultCenter]
        postNotificationName:NSApplicationWillFinishLaunchingNotification
        object:NSApp];
    [[NSNotificationCenter defaultCenter]
        postNotificationName:NSApplicationDidFinishLaunchingNotification
        object:NSApp];
}

- (void)run_step
{
    NSEvent *event;
    do{
        event = [self
                nextEventMatchingMask:NSAnyEventMask
                untilDate:nil
//                untilDate: [NSDate distantFuture]
                inMode:NSDefaultRunLoopMode
                dequeue:YES];
        [self sendEvent:event];
        [self updateWindows];
    }while(event != nil);
}

@end

////////////////////////////////////////////////////////////////////
// PangolinWindowDelegate
////////////////////////////////////////////////////////////////////

@interface PangolinWindowDelegate : NSObject <NSWindowDelegate>

@end

@implementation PangolinWindowDelegate

- (BOOL)windowShouldClose:(id)sender {
    pangolin::Quit();
    return YES;
}

@end

////////////////////////////////////////////////////////////////////
// PangolinAppDelegate
////////////////////////////////////////////////////////////////////

@interface PangolinAppDelegate : NSObject <NSApplicationDelegate>

@property (nonatomic, readonly) NSWindow *window;

@end

@implementation PangolinAppDelegate

@synthesize window = _window;

- (NSWindow*)window
{
    if (_window != nil)
        return(_window);
    
    NSRect viewRect = NSMakeRect(
        0.0, 0.0,
        pangolin::context->windowed_size[0],
        pangolin::context->windowed_size[1]
    );
    
    _window = [[NSWindow alloc] initWithContentRect:viewRect styleMask:NSTitledWindowMask|NSMiniaturizableWindowMask|NSResizableWindowMask|NSClosableWindowMask backing:NSBackingStoreBuffered defer:YES];
    [_window setTitle:[NSString stringWithUTF8String:window_name.c_str()]];
    [_window setOpaque:YES];
    [_window makeKeyAndOrderFront:NSApp];

    PangolinWindowDelegate *windelegate = [[PangolinWindowDelegate alloc] init];
    [_window setDelegate:windelegate];

    return(_window);
}

- (void)dealloc
{
    [_window dealloc];
    [super dealloc];
}

//- (void)setupMenu
//{
//    NSMenu *mainMenuBar;
//    NSMenu *appMenu;
//    NSMenuItem *menuItem;
    
//    mainMenuBar = [[NSMenu alloc] init];
    
//    appMenu = [[NSMenu alloc] initWithTitle:@"Pangolin Application"];
//    menuItem = [appMenu addItemWithTitle:@"Quit Pangolin Application" action:@selector(terminate:) keyEquivalent:@"q"];
//    [menuItem setKeyEquivalentModifierMask:NSCommandKeyMask];
    
//    menuItem = [[NSMenuItem alloc] init];
//    [menuItem setSubmenu:appMenu];
    
//    [mainMenuBar addItem:menuItem];
    
//    //[NSApp performSelector:@selector(setAppleMenu:) withObject:appMenu];
//    [appMenu release];
//    [NSApp setMainMenu:mainMenuBar];
//}

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
//    [self setupMenu];
    
    NSOpenGLPixelFormatAttribute attrs[] =
    {
        NSOpenGLPFADoubleBuffer,
        NSOpenGLPFADepthSize, 32,
//        NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion4_1Core,
        0
    };

    NSOpenGLPixelFormat *format = [[NSOpenGLPixelFormat alloc] initWithAttributes:attrs];
    view = [[PangolinNSGLView alloc] initWithFrame:self.window.frame pixelFormat:format];
    [format release];
#if MAC_OS_X_VERSION_MAX_ALLOWED >= 1070
    if (floor(NSAppKitVersionNumber) > NSAppKitVersionNumber10_6)
        [view setWantsBestResolutionOpenGLSurface:YES];
#endif /*MAC_OS_X_VERSION_MAX_ALLOWED*/

    [self.window setContentView:view];
}

@end

namespace pangolin
{

void FinishFrame()
{
    RenderViews();
    PostRender();
    [[view openGLContext] flushBuffer];
//    [[view openGLContext] update];
//    [view setNeedsDisplay:YES];
    [NSApp run_step];
}

void CreateWindowAndBind(std::string window_title, int w, int h )
{
    // Create Pangolin GL Context
    BindToContext(window_title);
    PangolinCommonInit();
    context->is_double_buffered = true;

//    // These are important I think!
//    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
//    [pool release];

    window_name = window_title;
    pangolin::context->windowed_size[0] = w;
    pangolin::context->windowed_size[1] = h;
    NSApp = [PangolinNSApplication sharedApplication];
    PangolinAppDelegate *delegate = [[PangolinAppDelegate alloc] init];

    [[PangolinNSApplication sharedApplication] setDelegate:delegate];

    [NSApp run_pre];
    [NSApp run_step];

    glewInit();
}

void StartFullScreen() {
}

void StopFullScreen() {
}

void SetFullscreen(bool fullscreen)
{
    if( fullscreen != context->is_fullscreen )
    {
        if(fullscreen) {
            StartFullScreen();
        }else{
            StopFullScreen();
        }
        context->is_fullscreen = fullscreen;
    }
}
}
