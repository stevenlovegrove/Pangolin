#include <pangolin/pangolin.h>
#include <pangolin/handler_glbuffer.h>
#include <pangolin/glsl.h>

#include "OculusHud.h"


int main(int argc, char ** argv) {
    pangolin::CreateWindowAndBind("Main",640,480);
    glEnable(GL_DEPTH_TEST);

    pangolin::OculusHud oculus;

    while( !pangolin::ShouldQuit() )
    {
        oculus.Bind();
        glClearColor(1,1,1,0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        for(unsigned int view = 0; view < oculus.NumViews(); ++view)
        {
            oculus.RenderToView(view);
            pangolin::glDrawColouredCube();
        }
        oculus.Unbind();

        pangolin::FinishFrame();
    }

    return 0;
}
