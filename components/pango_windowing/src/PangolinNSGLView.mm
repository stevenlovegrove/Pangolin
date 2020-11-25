#include <pangolin/platform.h>
#include <pangolin/gl/glinclude.h>
#include <pangolin/windowing/PangolinNSGLView.h>
#include <pangolin/windowing/handler_enums.h>

#if MAC_OS_X_VERSION_MAX_ALLOWED >= 101200
#  define NSDeviceIndependentModifierFlagsMask NSEventModifierFlagDeviceIndependentFlagsMask
#  define  NSShiftKeyMask NSEventModifierFlagShift
#  define  NSControlKeyMask NSEventModifierFlagControl
#  define  NSAlternateKeyMask NSEventModifierFlagOption
#  define  NSCommandKeyMask NSEventModifierFlagCommand
#  define  NSFunctionKeyMask NSEventModifierFlagFunction
#endif

////////////////////////////////////////////////////////////////////
// Input maps
////////////////////////////////////////////////////////////////////

inline
int mapMouseButton(int osx_button )
{
    const int map[] = {0, 2, 1, 3, 4, 5, 6, 7, 8, 9, 10};
    return map[osx_button];
}

inline
int mapKeymap(int osx_key)
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

////////////////////////////////////////////////////////////////////
// PangolinNSGLView
////////////////////////////////////////////////////////////////////

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
    if ( [self wantsBestResolutionOpenGLSurface] && [ self.window respondsToSelector:@selector(backingScaleFactor) ] )
        backing_scale = [self.window backingScaleFactor];
    else
#endif
    backing_scale = 1.0;

    const int width  = self.bounds.size.width * backing_scale;
    const int height = self.bounds.size.height * backing_scale;
    osx_window->ResizeSignal(pangolin::ResizeEvent({width, height}));

    [[self openGLContext] update];
    [super reshape];
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
        osx_window->KeyboardSignal(pangolin::KeyboardEvent(
            {(unsigned char)mapKeymap(osx_key), true, (float)location.x, (float)location.y}
        ));
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
        osx_window->KeyboardSignal(pangolin::KeyboardEvent(
            {(unsigned char)mapKeymap(osx_key), false, (float)location.x, (float)location.y}
        ));
    }
}

- (void)flagsChanged:(NSEvent *)event
{
    unsigned int flags = [event modifierFlags] & NSDeviceIndependentModifierFlagsMask;

    // TODO

//    if(flags&NSShiftKeyMask) {
//        context->mouse_state |=  pangolin::KeyModifierShift;
//    }else{
//        context->mouse_state &= ~pangolin::KeyModifierShift;
//    }

//    if(flags&NSControlKeyMask) {
//        context->mouse_state |=  pangolin::KeyModifierCtrl;
//    }else{
//        context->mouse_state &= ~pangolin::KeyModifierCtrl;
//    }

//    if(flags&NSAlternateKeyMask) {
//        context->mouse_state |=  pangolin::KeyModifierAlt;
//    }else{
//        context->mouse_state &= ~pangolin::KeyModifierAlt;
//    }

//    if(flags&NSCommandKeyMask) {
//        context->mouse_state |=  pangolin::KeyModifierCmd;
//    }else{
//        context->mouse_state &= ~pangolin::KeyModifierCmd;
//    }

//    if(flags&NSFunctionKeyMask) {
//        context->mouse_state |=  pangolin::KeyModifierFnc;
//    }else{
//        context->mouse_state &= ~pangolin::KeyModifierFnc;
//    }
}

////////////////////////////////////////////////////////////////////
// Mouse Input
////////////////////////////////////////////////////////////////////

-(void)mouseDownCommon:(NSEvent *)theEvent
{
    const int button = (int)[theEvent buttonNumber];
    const NSPoint location = [self _Location: theEvent];
    osx_window->MouseSignal(pangolin::MouseEvent({mapMouseButton(button), 0, (float)location.x, (float)location.y}));
}

-(void)mouseUpCommon:(NSEvent *)theEvent
{
    const int button = (int)[theEvent buttonNumber];
    const NSPoint location = [self _Location: theEvent];
    osx_window->MouseSignal(pangolin::MouseEvent({mapMouseButton(button), 1, (float)location.x, (float)location.y}));
}

- (void)mouseDraggedCommon: (NSEvent *)theEvent
{
    const NSPoint location = [self _Location: theEvent];
    osx_window->MouseMotionSignal(pangolin::MouseMotionEvent({(float)location.x, (float)location.y}));
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
    osx_window->PassiveMouseMotionSignal(pangolin::MouseMotionEvent({(float)location.x, (float)location.y}));
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
        osx_window->SpecialInputSignal(pangolin::SpecialInputEvent({
            pangolin::InputSpecialScroll,
            (float)location.x, (float)location.y,
            {dx, dy, 0.0f, 0.0f}
        }));
    }
}

- (void)magnifyWithEvent: (NSEvent *)theEvent
{
    const NSPoint location = [self _Location: theEvent];
    const float dm = theEvent.magnification;
    if(dm != 0.0f) {
        osx_window->SpecialInputSignal(pangolin::SpecialInputEvent({
            pangolin::InputSpecialZoom,
            (float)location.x, (float)location.y,
            {dm, 0.0f, 0.0f, 0.0f}
        }));
    }
}

- (void)rotateWithEvent: (NSEvent *)theEvent
{
    const NSPoint location = [self _Location: theEvent];
    const float dr = theEvent.rotation;
    if(dr != 0.0f) {
        osx_window->SpecialInputSignal(pangolin::SpecialInputEvent({
            pangolin::InputSpecialRotate,
            (float)location.x, (float)location.y,
            {dr, 0.0f, 0.0f, 0.0f}
        }));
    }
}

- (void)mouseEntered: (NSEvent *)theEvent
{
    PANGOLIN_UNUSED(theEvent);
}

- (void)mouseExited: (NSEvent *)theEvent
{
    PANGOLIN_UNUSED(theEvent);
}

-(void)dealloc
{
    [super dealloc];
}

@end
