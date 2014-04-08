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

#ifndef PANGOLIN_DISPLAY_H
#define PANGOLIN_DISPLAY_H

#include <pangolin/platform.h>
#include <pangolin/glinclude.h>
#include <pangolin/compat/function.h>
#include <pangolin/handler_enums.h>

#include <string>

namespace pangolin
{

  // Forward Declarations
  struct View;
  struct Viewport;
  class  UserApp;
  
  //! @brief Give this OpenGL context a name or switch contexts
  //! This is required to initialise Pangolin for use with an
  //! externally defined OpenGL context. You needn't call it
  //! if you have used CreateWindowAndBind() to create a window
  //! or launched a pangolin::UserApp
  PANGOLIN_EXPORT
  void BindToContext(std::string name);

  //! @brief Initialise OpenGL window (determined by platform) and bind context
  //! This method will choose an available windowing system if one is present
  //! Currently, uses GLUT on a desktop, and EGL on android.
  PANGOLIN_EXPORT
  void CreateWindowAndBind(std::string window_title, int w = 640, int h = 480);

  //! @brief Launch users derived UserApp, controlling OpenGL event loop
  //! This method will block until the application exits, calling app's
  //! Init() method to start and Render() method subsequently to draw each frame.
  //! @return exit code for use when returning from main. Currently always 0.
  PANGOLIN_EXPORT
  int LaunchUserApp(UserApp& app);
  
  //! @brief Perform any post rendering, event processing and frame swapping
  PANGOLIN_EXPORT
  void FinishFrame();

  //! @brief Request that the program exit
  PANGOLIN_EXPORT
  void Quit();

  //! @brief Returns true if user has requested to close OpenGL window.
  PANGOLIN_EXPORT
  bool ShouldQuit();

  //! @brief Returns true if user has interacted with the window since this was last called
  PANGOLIN_EXPORT
  bool HadInput();

  //! @brief Returns true if user has resized the window
  PANGOLIN_EXPORT
  bool HasResized();

  //! @brief Renders any views with default draw methods
  PANGOLIN_EXPORT
  void RenderViews();
  
  //! @brief Perform any post render events, such as screen recording.
  PANGOLIN_EXPORT
  void PostRender();

  //! @brief Request to be notified via functor when key is pressed.
  //! Functor may take one parameter which will equal the key pressed
  PANGOLIN_EXPORT
  void RegisterKeyPressCallback(int key, boostd::function<void(void)> func);

  //! @brief Save window contents to image
  PANGOLIN_EXPORT
  void SaveWindowOnRender(std::string filename_prefix);
  
  PANGOLIN_EXPORT
  void SaveFramebuffer(std::string prefix, const Viewport& v);
  
  namespace process
  {
    //! @brief Tell pangolin to process input to drive display
    //! You will need to call this manually if you haven't let
    //! Pangolin register callbacks from your windowing system
    PANGOLIN_EXPORT
    void Keyboard( unsigned char key, int x, int y);

    PANGOLIN_EXPORT
    void KeyboardUp(unsigned char key, int x, int y);
    
    PANGOLIN_EXPORT
    void SpecialFunc(int key, int x, int y);
    
    PANGOLIN_EXPORT
    void SpecialFuncUp(int key, int x, int y);

    //! @brief Tell pangolin base window size has changed
    //! You will need to call this manually if you haven't let
    //! Pangolin register callbacks from your windowing system
    PANGOLIN_EXPORT
    void Resize(int width, int height);

    //! @brief Event based rendering entry point (from e.g.
    //! glutMainLoop). Not currently supported.
    PANGOLIN_EXPORT
    void Display();

    PANGOLIN_EXPORT
    void Mouse( int button, int state, int x, int y);

    PANGOLIN_EXPORT
    void MouseMotion( int x, int y);

    PANGOLIN_EXPORT
    void PassiveMouseMotion(int x, int y);

    PANGOLIN_EXPORT
    void Scroll(float x, float y);

    PANGOLIN_EXPORT
    void Zoom(float m);

    PANGOLIN_EXPORT
    void Rotate(float r);
    
    PANGOLIN_EXPORT
    void SubpixMotion(float x, float y, float pressure, float rotation, float tiltx, float tilty);

    PANGOLIN_EXPORT
    void SpecialInput(InputSpecial inType, float x, float y, float p1, float p2, float p3, float p4);

  }

  //! Retrieve 'base' display, corresponding to entire window
  PANGOLIN_EXPORT
  View& DisplayBase();

  //! Create or retrieve named display managed by pangolin (automatically deleted)
  PANGOLIN_EXPORT
  View& Display(const std::string& name);

  //! Create unnamed display managed by pangolin (automatically deleted)
  PANGOLIN_EXPORT
  View& CreateDisplay();

  //! Switch between windowed and fullscreen mode
  PANGOLIN_EXPORT
  void ToggleFullscreen();

  //! Switch windows/fullscreenmode = fullscreen
  PANGOLIN_EXPORT
  void SetFullscreen(bool fullscreen = true);

}

#endif // PANGOLIN_DISPLAY_H

