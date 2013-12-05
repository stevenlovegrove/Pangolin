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
#include <pangolin/handler.h>
#include <pangolin/compat/function.h>

#include <string>

namespace pangolin
{

  // Forward Declarations
  struct View;
  struct Viewport;
  
  //! @brief Give this OpenGL context a name or switch contexts
  //! This is required to initialise Pangolin for use with an
  //! externally defined OpenGL context. You needn't call it
  //! if you have used CreateGlutWindowAndBind() to create a GLUT
  //! window
  void BindToContext(std::string name);

  //! @brief Initialise OpenGL window (determined by platform) and bind context
  //! This method will choose an available windowing system if one is present
  //! Currently, uses GLUT on a desktop, and EGL on android.
  void CreateWindowAndBind(std::string window_title, int w = 640, int h = 480);
  
  //! @brief Perform any post rendering, event processing and frame swapping
  void FinishFrame();

  //! @brief Request that the program exit
  void Quit();

  //! @brief Returns true if user has requested to close OpenGL window.
  bool ShouldQuit();

  //! @brief Returns true if user has interacted with the window since this was last called
  bool HadInput();

  //! @brief Returns true if user has resized the window
  bool HasResized();

  //! @brief Renders any views with default draw methods
  void RenderViews();
  
  //! @brief Perform any post render events, such as screen recording.
  void PostRender();

  //! @brief Request to be notified via functor when key is pressed.
  //! Functor may take one parameter which will equal the key pressed
  void RegisterKeyPressCallback(int key, boostd::function<void(void)> func);

  //! @brief Save window contents to image
  void SaveWindowOnRender(std::string filename_prefix);
  
  void SaveFramebuffer(std::string prefix, const Viewport& v);
  
  namespace process
  {
    //! @brief Tell pangolin to process input to drive display
    //! You will need to call this manually if you haven't let
    //! Pangolin register callbacks from your windowing system
    void Keyboard( unsigned char key, int x, int y);

    void KeyboardUp(unsigned char key, int x, int y);
    
    void SpecialFunc(int key, int x, int y);
    
    void SpecialFuncUp(int key, int x, int y);

    //! @brief Tell pangolin base window size has changed
    //! You will need to call this manually if you haven't let
    //! Pangolin register callbacks from your windowing system
    void Resize(int width, int height);

    void Mouse( int button, int state, int x, int y);

    void MouseMotion( int x, int y);

    void PassiveMouseMotion(int x, int y);

    void Scroll(float x, float y);

    void Zoom(float m);

    void Rotate(float r);
    
    void SubpixMotion(float x, float y, float pressure, float rotation, float tiltx, float tilty);    

    void SpecialInput(InputSpecial inType, float x, float y, float p1, float p2, float p3, float p4);

  }

  //! Retrieve 'base' display, corresponding to entire window
  View& DisplayBase();

  //! Create or retrieve named display managed by pangolin (automatically deleted)
  View& Display(const std::string& name);

  //! Create unnamed display managed by pangolin (automatically deleted)
  View& CreateDisplay();

}

#endif // PANGOLIN_DISPLAY_H

