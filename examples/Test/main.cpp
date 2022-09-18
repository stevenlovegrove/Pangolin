#include <pangolin/display/display.h>
#include <pangolin/display/default_font.h>
#include <pangolin/var/var.h>
#include <pangolin/display/widgets.h>

#include <pangolin/gl/gldraw.h>
#include <pangolin/gl/glvbo.h>
#include <pangolin/gl/glsl.h>
#include <pangolin/gl/glfont.h>
#include <pangolin/var/varextra.h>

#include "widget_panel.h"

using namespace pangolin;

void MainWidgets()
{
    using namespace pangolin;

    pangolin::CreateWindowAndBind("Pango GL Triangle With VBO and Shader", 640, 480, {{PARAM_GL_PROFILE, "3.2 CORE"}});
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    WidgetPanel panel;
    panel.SetBounds(0.0, 1.0, 0.0, 0.5);

    DisplayBase().AddDisplay(panel);

    while( !pangolin::ShouldQuit() )
    {
        glClearColor(0.7, 0.7, 0.7, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        pangolin::FinishFrame();
    }
}

int main( int /*argc*/, char** /*argv*/ )
{
    MainWidgets();
    return 0;
}
