#include <iostream>

#include <pangolin/var/var.h>
#include <pangolin/var/varextra.h>
#include <pangolin/gl/gl.h>
#include <pangolin/gl/gldraw.h>
#include <pangolin/display/display.h>
#include <pangolin/display/view.h>
#include <pangolin/display/widgets.h>
#include <pangolin/display/default_font.h>
#include <pangolin/handler/handler.h>

int main(/*int argc, char* argv[]*/)
{  
  // Create OpenGL window in single line
  pangolin::CreateWindowAndBind("Main",640,480);
  
  // 3D Mouse handler requires depth testing to be enabled
  glEnable(GL_DEPTH_TEST);

  // Define Camera Render Object (for view / scene browsing)
  pangolin::OpenGlRenderState s_cam(
    pangolin::ProjectionMatrix(640,480,420,420,320,240,0.1,1000),
    pangolin::ModelViewLookAt(-0,0.5,-3, 0,0,0, pangolin::AxisY)
  );

  // Choose a sensible left UI Panel width based on the width of 20
  // charectors from the default font.
  const int UI_WIDTH = 20* pangolin::default_font().MaxWidth();

  // Add named OpenGL viewport to window and provide 3D Handler
  pangolin::View& d_cam = pangolin::CreateDisplay()
    .SetBounds(0.0, 1.0, pangolin::Attach::Pix(UI_WIDTH), 1.0, 640.0f/480.0f)
    .SetHandler(new pangolin::Handler3D(s_cam));

  // Add named Panel and bind to variables beginning 'ui'
  // A Panel is just a View with a default layout and input handling
  pangolin::CreatePanel("ui")
      .SetBounds(0.0, 1.0, 0.0, pangolin::Attach::Pix(UI_WIDTH));

  // Safe and efficient binding of named variables.
  // Specialisations mean no conversions take place for exact types
  // and conversions between scalar types are cheap.
  pangolin::Var<bool> a_button("ui.A_Button",false,false);
  pangolin::Var<double> a_double("ui.A_Double",3,0,5);
  pangolin::Var<int> an_int("ui.An_Int",2,0,5);
  pangolin::Var<double> a_double_log("ui.Log_scale",3,1,1E4, true);
  pangolin::Var<bool> a_checkbox("ui.A_Checkbox",false,true);
  pangolin::Var<int> an_int_no_input("ui.An_Int_No_Input",2);
  pangolin::Var<std::string> a_string("ui.A_String", "Edit ME!");

  // std::function objects can be used for Var's too. These work great with C++11 closures.
  pangolin::Var<std::function<void(void)>> save_window("ui.Save_Window", [](){
      pangolin::SaveWindowOnRender("window");
  });

  pangolin::Var<std::function<void(void)>> save_cube_view("ui.Save_Cube", [&d_cam](){
      pangolin::SaveWindowOnRender("cube", d_cam.v);
  });

  // Demonstration of how we can register a keyboard hook to alter a Var
  pangolin::RegisterKeyPressCallback(pangolin::PANGO_CTRL + 'b', [&](){
      a_double = 3.5;
  });

  // Default hooks for exiting (Esc) and fullscreen (tab).
  while( !pangolin::ShouldQuit() )
  {
    // Clear entire screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);    

    if( pangolin::Pushed(a_button) )
      std::cout << "You Pushed a button!" << std::endl;

    // Overloading of Var<T> operators allows us to treat them like
    // their wrapped types, eg:
    if( a_checkbox )
      an_int = (int)a_double;

    an_int_no_input = an_int;

    if(d_cam.IsShown()) {
        // Activate efficiently by object
        d_cam.Activate(s_cam);

        // Render some stuff
        glColor3f(1.0,1.0,1.0);
        pangolin::glDrawColouredCube();
    }

    // Swap frames and Process Events
    pangolin::FinishFrame();
  }

  return 0;
}
