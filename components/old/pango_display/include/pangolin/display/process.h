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

#pragma once

#include <pangolin/platform.h>
#include <pangolin/windowing/handler_bitsets.h>

namespace pangolin
{

/// You can use these methods to drive Pangolin events when it
/// doesn't own the OpenGL Context. You can probably ignore these.
namespace process
{
  PANGOLIN_EXPORT
  void Resize(int width, int height);

  PANGOLIN_EXPORT
  void Keyboard(unsigned char key, int x, int y, bool pressed, KeyModifierBitmask key_modifiers);

  PANGOLIN_EXPORT
  void Mouse(int button, bool pressed, int x, int y, KeyModifierBitmask key_modifiers);

  PANGOLIN_EXPORT
  void MouseMotion( int x, int y, KeyModifierBitmask key_modifiers);

  PANGOLIN_EXPORT
  void PassiveMouseMotion(int x, int y, KeyModifierBitmask key_modifiers);

  PANGOLIN_EXPORT
  void SpecialInput(InputSpecial inType, float x, float y, float p1, float p2, float p3, float p4, KeyModifierBitmask key_modifiers);
}

}
