#include <pangolin/pangolin.h>

int main( int /*argc*/, char** /*argv*/ )
{
    const int w = 512;
    const int h = 512;

    const int UI_WIDTH = 180;
    pangolin::CreateWindowAndBind("Main",UI_WIDTH+w,h);
    glEnable(GL_DEPTH_TEST);

    pangolin::CreatePanel("ui")
        .SetBounds(0.0, 1.0, 0.0, pangolin::Attach::Pix(UI_WIDTH));

    Eigen::Matrix4d orth = pangolin::ProjectionMatrixOrthographic(-2,2,-2,2,0.2,100);
    Eigen::Matrix4d proj = pangolin::ProjectionMatrix(w,h,420,420,w/2.0,h/2.0,0.2,100);
    Eigen::Matrix4d curr = proj;

    // Define Projection and initial ModelView matrix
    pangolin::OpenGlRenderState s_cam(
        proj,
        pangolin::ModelViewLookAt(-2,2,-2, 0,0,0, pangolin::AxisY)
    );

    // Create Interactive View in window
    pangolin::Handler3D handler(s_cam);
    pangolin::View& d_cam = pangolin::CreateDisplay()
            .SetBounds(0.0, 1.0, pangolin::Attach::Pix(UI_WIDTH), 1.0, -w/(double)h)
            .SetHandler(&handler);

    Eigen::Matrix4d* target = &proj;

    pangolin::RegisterKeyPressCallback('p', [&](){
        target = &proj;
    });
    pangolin::RegisterKeyPressCallback('o', [&](){
        target = &orth;
    });

    pangolin::Var<double> factor("ui.factor", 1.0);

    while( !pangolin::ShouldQuit() )
    {
        curr = factor*proj + (1.0-factor)*orth;
        s_cam.GetProjectionMatrix() = curr;

        // Clear screen and activate view to render into
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        d_cam.Activate(s_cam);

        // Render OpenGL Cube
        pangolin::glDrawColouredCube();

        // Swap frames and Process Events
        pangolin::FinishFrame();
    }

    return 0;
}
