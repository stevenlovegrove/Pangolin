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

inline
pangolin::KeyModifierBitmask GetKeyModifierBitmask(NSEvent *event)
{
    unsigned int flags = [event modifierFlags] & NSDeviceIndependentModifierFlagsMask;
    pangolin::KeyModifierBitmask mask;
    if(flags&NSShiftKeyMask) mask |=  pangolin::KeyModifierShift;
    if(flags&NSControlKeyMask) mask |=  pangolin::KeyModifierCtrl;
    if(flags&NSAlternateKeyMask) mask |=  pangolin::KeyModifierAlt;
    if(flags&NSCommandKeyMask) mask |=  pangolin::KeyModifierCmd;
    if(flags&NSFunctionKeyMask) mask |=  pangolin::KeyModifierFnc;
    return mask;
}

////////////////////////////////////////////////////////////////////
// PangolinNSGLView
////////////////////////////////////////////////////////////////////

@implementation PangolinNSGLView

- (void)updateTrackingAreas {
  [self initTrackingArea];
}

- (void) initTrackingArea {
  NSTrackingAreaOptions options = (NSTrackingActiveAlways | NSTrackingInVisibleRect |
                                   NSTrackingMouseEnteredAndExited | NSTrackingMouseMoved);

  NSTrackingArea *area = [[NSTrackingArea alloc] initWithRect:[self bounds]
                                                      options:options
                                                        owner:self
                                                     userInfo:nil];
  [self addTrackingArea:area];
  [area release];
}

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
    osx_window->ResizeSignal(pangolin::WindowResizeEvent({width, height}));

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

-(NSPoint)_Location:(NSEvent *)event
{
    NSPoint location = [self convertPoint: [event locationInWindow] fromView: nil];
    location.x *= backing_scale;
    location.y *= backing_scale;
    return location;
}

////////////////////////////////////////////////////////////////////
// Keyboard
////////////////////////////////////////////////////////////////////

-(void)keyDown:(NSEvent *)event
{
    const NSPoint location = [self _Location: event];
    NSString *str = [event characters];
    int len = (int)[str length];
    for(int i = 0; i < len; i++)
    {
        const int osx_key = [str characterAtIndex:i];
        osx_window->KeyboardSignal(pangolin::KeyboardEvent(
            {(float)location.x, (float)location.y,
             GetKeyModifierBitmask(event),
             (unsigned char)mapKeymap(osx_key), true}
        ));
    }
}

-(void)keyUp:(NSEvent *)event
{
    const NSPoint location = [self _Location: event];
    NSString *str = [event characters];
    int len = (int)[str length];
    for(int i = 0; i < len; i++)
    {
        const int osx_key = [str characterAtIndex:i];
        osx_window->KeyboardSignal(pangolin::KeyboardEvent(
            {(float)location.x, (float)location.y,
             GetKeyModifierBitmask(event),
             (unsigned char)mapKeymap(osx_key), false}
        ));
    }
}

////////////////////////////////////////////////////////////////////
// Mouse Input
////////////////////////////////////////////////////////////////////

-(void)mouseDownCommon:(NSEvent *)event
{
    const int button = (int)[event buttonNumber];
    const NSPoint location = [self _Location: event];
    osx_window->MouseSignal(pangolin::MouseEvent({
        (float)location.x, (float)location.y,
        GetKeyModifierBitmask(event),
        mapMouseButton(button), true
    }));
}

-(void)mouseUpCommon:(NSEvent *)event
{
    const int button = (int)[event buttonNumber];
    const NSPoint location = [self _Location: event];
    osx_window->MouseSignal(pangolin::MouseEvent({
        (float)location.x, (float)location.y,
        GetKeyModifierBitmask(event),
        mapMouseButton(button), false
    }));
}

- (void)mouseDraggedCommon: (NSEvent *)event
{
    const NSPoint location = [self _Location: event];
    osx_window->MouseMotionSignal(pangolin::MouseMotionEvent({
        (float)location.x, (float)location.y,
        GetKeyModifierBitmask(event)
    }));
}

-(void)mouseDown:(NSEvent *)event
{
    [self mouseDownCommon:event];
}

-(void)mouseUp:(NSEvent *)event
{
    [self mouseUpCommon:event];
}

- (void)mouseDragged: (NSEvent *)event
{
    [self mouseDraggedCommon:event];
}

-(void)rightMouseDown:(NSEvent *)event
{
    [self mouseDownCommon:event];
}

-(void)rightMouseUp:(NSEvent *)event
{
    [self mouseUpCommon:event];
}

- (void)rightMouseDragged: (NSEvent *)event
{
    [self mouseDraggedCommon:event];
}

-(void)otherMouseDown:(NSEvent *)event
{
    [self mouseDownCommon:event];
}

-(void)otherMouseUp:(NSEvent *)event
{
    [self mouseUpCommon:event];
}

- (void)otherMouseDragged: (NSEvent *)event
{
    [self mouseDraggedCommon:event];
}

- (void)mouseMoved: (NSEvent *)event
{
    const NSPoint location = [self _Location: event];
    osx_window->PassiveMouseMotionSignal(pangolin::MouseMotionEvent({(float)location.x, (float)location.y}));
}

- (void)scrollWheel:(NSEvent *)event
{
    const NSPoint location = [self _Location: event];

    float dx, dy;
    if([event respondsToSelector:@selector(scrollingDeltaX)]) {
       dx = event.scrollingDeltaX; dy = event.scrollingDeltaY;
    } else {
       dx = event.deltaX; dy = event.deltaY;
    }

    if(dx != 0.0f || dy != 0.0f) {
        osx_window->SpecialInputSignal(pangolin::SpecialInputEvent({
            (float)location.x, (float)location.y,
            GetKeyModifierBitmask(event),
            pangolin::InputSpecialScroll,
            {dx, dy, 0.0f, 0.0f}
        }));
    }
}

- (void)magnifyWithEvent: (NSEvent *)event
{
    const NSPoint location = [self _Location: event];
    const float dm = event.magnification;
    if(dm != 0.0f) {
        osx_window->SpecialInputSignal(pangolin::SpecialInputEvent({
            (float)location.x, (float)location.y,
            GetKeyModifierBitmask(event),
            pangolin::InputSpecialZoom,
            {dm, 0.0f, 0.0f, 0.0f}
        }));
    }
}

- (void)rotateWithEvent: (NSEvent *)event
{
    const NSPoint location = [self _Location: event];
    const float dr = event.rotation;
    if(dr != 0.0f) {
        osx_window->SpecialInputSignal(pangolin::SpecialInputEvent({
            (float)location.x, (float)location.y,
            GetKeyModifierBitmask(event),
            pangolin::InputSpecialRotate,
            {dr, 0.0f, 0.0f, 0.0f}
        }));
    }
}

- (void)mouseEntered: (NSEvent *)event
{
    PANGOLIN_UNUSED(event);
}

- (void)mouseExited: (NSEvent *)event
{
    PANGOLIN_UNUSED(event);
}

-(void)dealloc
{
    [super dealloc];
}

@end
