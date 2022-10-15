#include <pangolin/display/display.h>
#include <pangolin/display/default_font.h>
#include <pangolin/var/var.h>
#include <pangolin/display/widget_panel.h>

#include <pangolin/gl/gldraw.h>
#include <pangolin/gl/glvbo.h>
#include <pangolin/gl/glsl.h>
#include <pangolin/gl/glfont.h>
#include <pangolin/var/varextra.h>

using namespace pangolin;

void MainWidgets()
{
    using namespace pangolin;

    // pangolin::CreateWindowAndBind("Pango GL Triangle With VBO and Shader", 640, 480, {{PARAM_GL_PROFILE, "3.2 CORE"}});
    pangolin::CreateWindowAndBind("Pango GL Triangle With VBO and Shader", 640, 480);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    WidgetPanel panel;
    panel.SetBounds(0.0, 1.0, 0.0, 0.3);

    DisplayBase().AddDisplay(panel);

    Var<float> test1("ui.slider1", 20.0, 0.0, 50.0);
    Var<int> test2("ui.slider2", 3, 0, 15);
    Var<double> test3("ui.slider3", 3.2e-1, 0.0, 1.0);
    Var<bool> test4("ui.button", false, false);
    Var<bool> test5("ui.checkbox", false, true);
    Var<std::function<void(void)>> test6("ui.button2", [](){
        static int i=0;
        std::cout << "Hello " << i++ << std::endl;
    });
    Var<std::string> test7("ui.textbox", "Hello");

    while( !pangolin::ShouldQuit() )
    {
        glClearColor(0.7, 0.7, 0.7, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

//        std::cout << test1 << ", " << test2 << std::endl;

        pangolin::FinishFrame();
    }
}

int main( int /*argc*/, char** /*argv*/ )
{
    MainWidgets();
    return 0;
}
