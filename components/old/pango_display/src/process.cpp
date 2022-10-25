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

#include <pangolin/display/process.h>
#include <pangolin/console/ConsoleView.h>
#include <pangolin/handler/handler.h>
#include "pangolin_gl.h"

namespace pangolin
{
namespace process
{
float last_x = 0;
float last_y = 0;

void Resize( int width, int height )
{
    PangolinGl* context = GetCurrentContext();

    Viewport win(0,0,width,height);
    context->base.Resize(win);
}

void Keyboard(unsigned char key, int x, int y, bool pressed, KeyModifierBitmask /*key_modifiers*/)
{
    PangolinGl* context = GetCurrentContext();

    // Force coords to match OpenGl Window Coords
    y = context->base.v.h - y;

    if(pressed) {
        // Check if global key hook exists
        const KeyhookMap::iterator hook = context->keypress_hooks.find(key);

        // Console receives all input when it is open
        if( context->console_view && context->console_view->IsShown() ) {
            context->console_view->Keyboard(*(context->console_view),key,x,y,true);
        }else if(hook != context->keypress_hooks.end() ) {
            hook->second(key);
        } else if(context->activeDisplay && context->activeDisplay->handler) {
            context->activeDisplay->handler->Keyboard(*(context->activeDisplay),key,x,y,true);
        }else{
            context->base.handler->Keyboard(context->base, key,x,y,true);
        }
    }else{
        if(context->activeDisplay && context->activeDisplay->handler)
        {
            context->activeDisplay->handler->Keyboard(*(context->activeDisplay),key,x,y,false);
        }else{
            context->base.handler->Keyboard(context->base, key,x,y,false);
        }
    }
}

void Mouse( int button_raw, bool pressed, int x, int y, KeyModifierBitmask key_modifiers)
{
    PangolinGl* context = GetCurrentContext();

    // Force coords to match OpenGl Window Coords
    y = context->base.v.h - y;

    last_x = (float)x;
    last_y = (float)y;

    const MouseButton button = (MouseButton)(1 << (button_raw & 0xf) );

    const bool fresh_input = ( (context->mouse_state & 7) == 0);

    if( pressed ) {
        context->mouse_state |= (button&7);
    }else{
        context->mouse_state &= ~(button&7);
    }

#if defined(_WIN_)
    context->mouse_state &= 0x0000ffff;
    context->mouse_state |= (button_raw >> 4) << 16;
#endif

    const int button_state = context->mouse_state | key_modifiers.mask();

    if(fresh_input) {
        context->base.handler->Mouse(context->base,button,x,y,pressed,button_state);
    }else if(context->activeDisplay && context->activeDisplay->handler) {
        context->activeDisplay->handler->Mouse(*(context->activeDisplay),button,x,y,pressed,button_state);
    }
}

void MouseMotion( int x, int y, KeyModifierBitmask key_modifiers)
{
    PangolinGl* context = GetCurrentContext();

    // Force coords to match OpenGl Window Coords
    y = context->base.v.h - y;

    last_x = (float)x;
    last_y = (float)y;

    const int button_state = context->mouse_state | key_modifiers.mask();

    if( context->activeDisplay)
    {
        if( context->activeDisplay->handler )
            context->activeDisplay->handler->MouseMotion(*(context->activeDisplay),x,y,button_state);
    }else{
        context->base.handler->MouseMotion(context->base,x,y,button_state);
    }
}

void PassiveMouseMotion(int x, int y, KeyModifierBitmask key_modifiers)
{
    PangolinGl* context = GetCurrentContext();

    // Force coords to match OpenGl Window Coords
    y = context->base.v.h - y;

    const int button_state = context->mouse_state | key_modifiers.mask();
    context->base.handler->PassiveMouseMotion(context->base,x,y,button_state);

    last_x = (float)x;
    last_y = (float)y;
}

void SpecialInput(InputSpecial inType, float x, float y, float p1, float p2, float p3, float p4, KeyModifierBitmask key_modifiers)
{
    PangolinGl* context = GetCurrentContext();

    // Force coords to match OpenGl Window Coords
    y = context->base.v.h - y;

    const bool fresh_input = (context->mouse_state == 0);
    const int button_state = context->mouse_state | key_modifiers.mask();

    if(fresh_input) {
        context->base.handler->Special(context->base,inType,x,y,p1,p2,p3,p4,button_state);
    }else if(context->activeDisplay && context->activeDisplay->handler) {
        context->activeDisplay->handler->Special(*(context->activeDisplay),inType,x,y,p1,p2,p3,p4,button_state);
    }
}
}
}
