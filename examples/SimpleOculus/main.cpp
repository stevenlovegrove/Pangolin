#include <pangolin/pangolin.h>
#include <pangolin/hud/oculus_hud.h>

int main(int argc, char ** argv) {
    pangolin::CreateWindowAndBind("Main",640,480);
    glEnable(GL_DEPTH_TEST);

    pangolin::OculusHud oculus;
    pangolin::OpenGlRenderState s_cam = oculus.DefaultRenderState();
    s_cam.SetModelViewMatrix(pangolin::ModelViewLookAt(-1.5,1.5,-1.5, 0,0,0, pangolin::AxisY));
    oculus.SetHandler(new pangolin::Handler3DOculus(oculus, s_cam));

    while( !pangolin::ShouldQuit() )
    {
        // Update modelview matrix with head transform
        s_cam.GetModelViewMatrix() = oculus.HeadTransformDelta() * s_cam.GetModelViewMatrix();

        oculus.Framebuffer().Bind();
        glClearColor(1,1,1,0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        for(unsigned int view = 0; view < oculus.NumChildren(); ++view)
        {
            oculus[view].Activate();
            s_cam.ApplyNView(view);
            pangolin::glDrawColouredCube();
        }
        oculus.Framebuffer().Unbind();

        pangolin::FinishFrame();
    }

    return 0;
}
