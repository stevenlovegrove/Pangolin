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
#include <pangolin/gl/glinclude.h>
#include <pangolin/gl/viewport.h>
#include <pangolin/utils/params.h>
#include <pangolin/windowing/window.h>

#include <functional>
#include <string>
#include <memory>

/*! \file display.h
 * This file contains a number of global methods for creating and
 * querying window state as well as handling user input.
 */

namespace pangolin
{

  // Forward Declarations
  struct View;
  class  UserApp;
  
  /// Give this OpenGL context a name or switch contexts.
  /// This is required to initialise Pangolin for use with an
  /// externally defined OpenGL context. You needn't call it
  /// if you have used CreateWindowAndBind() to create a window
  /// or launched a pangolin::UserApp
  PANGOLIN_EXPORT
  void BindToContext(std::string name);

  /// Initialise OpenGL window (determined by platform) and bind context.
  /// This method will choose an available windowing system if one is present.
  PANGOLIN_EXPORT
  WindowInterface& CreateWindowAndBind(std::string window_title, int w = 640, int h = 480, const Params& params = Params());

  /// Return pointer to current Pangolin Window context, or nullptr if none bound.
  PANGOLIN_EXPORT
  WindowInterface* GetBoundWindow();

  PANGOLIN_EXPORT
  void DestroyWindow(const std::string& window_title);

  /// Launch users derived UserApp, controlling OpenGL event loop.
  /// This method will block until the application exits, calling app's
  /// Init() method to start and Render() method subsequently to draw each frame.
  /// @return exit code for use when returning from main. Currently always 0.
  PANGOLIN_EXPORT
  int LaunchUserApp(UserApp& app);
  
  /// Perform any post rendering, event processing and frame swapping.
  PANGOLIN_EXPORT
  void FinishFrame();

  /// Request that the window close.
  PANGOLIN_EXPORT
  void Quit();

  /// Request that all windows close.
  PANGOLIN_EXPORT
  void QuitAll();

  /// Returns true if user has requested to close OpenGL window.
  PANGOLIN_EXPORT
  bool ShouldQuit();

  /// Renders any views with default draw methods.
  PANGOLIN_EXPORT
  void RenderViews();
  
  /// Perform any post render events, such as screen recording.
  PANGOLIN_EXPORT
  void PostRender();

  /// Request to be notified via functor when key is pressed.
  PANGOLIN_EXPORT
  void RegisterKeyPressCallback(int key, std::function<void(void)> func);

  /// Request to be notified via functor when key is pressed.
  /// Functor may take one parameter which will equal the key pressed
  PANGOLIN_EXPORT
  void RegisterKeyPressCallback(int key, std::function<void(int)> func);

  /// Save the contents of current window within the specified viewport (whole window by default).
  /// This will be called during pangolin::FinishFrame().
  /// \param filename_hint can be a complete filename (absolute or relative to working directory).
  /// \param the portion of the window to save. Default construction will save entire window.
  PANGOLIN_EXPORT
  void SaveWindowOnRender(const std::string& filename_hint, const Viewport& v = Viewport());

  /// Save the contents of current window within the specified viewport (whole window by default).
  /// This will block whilst waiting for pending draw calls to complete and then save the current contents.
  /// \param filename_hint can be a complete filename (absolute or relative to working directory).
  /// \param the portion of the window to save. Default construction will save entire window.
  PANGOLIN_EXPORT
  void SaveWindowNow(const std::string& filename_hint, const Viewport& v = Viewport());
  
  /// Retrieve 'base' display, corresponding to entire window.
  PANGOLIN_EXPORT
  View& DisplayBase();

  /// Create or retrieve named display managed by pangolin (automatically deleted).
  PANGOLIN_EXPORT
  View& Display(const std::string& name);

  /// Create unnamed display managed by pangolin (automatically deleted).
  PANGOLIN_EXPORT
  View& CreateDisplay();

  /// Switch windows/fullscreenmode = fullscreen.
  PANGOLIN_EXPORT
  void ShowFullscreen(TrueFalseToggle on_off);

  /// Toggle display of Pangolin console
  PANGOLIN_EXPORT
  void ShowConsole(TrueFalseToggle on_off);
}

#include <pangolin/display/display.hpp>
