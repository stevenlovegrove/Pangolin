#include <pangolin/pangolin.h>

int main( int /*argc*/, char** /*argv*/ )
{  
    pangolin::CreateGlutWindowAndBind("Main",640,480);
    glEnable(GL_DEPTH_TEST);
    
    // Define Projection and initial ModelView matrix
    pangolin::OpenGlRenderState s_cam(
        pangolin::ProjectionMatrix(640,480,420,420,320,240,0.1,1000),
        pangolin::ModelViewLookAt(-0,0.5,-3, 0,0,0, pangolin::AxisY)
    );
    
    // Create Interactive View in window
    pangolin::View& d_cam = pangolin::CreateDisplay()
            .SetBounds(0.0, 1.0, 0.0, 1.0, -640.0f/480.0f)
            .SetHandler(new pangolin::Handler3D(s_cam));
    
    while( !pangolin::ShouldQuit() )
    {
        // Clear screen and activate view to render into
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        d_cam.Activate(s_cam);
        
        // Render OpenGL Teapot
        glColor3f(1.0,1.0,1.0);
        glutWireTeapot(1.0);
        
        // Swap frames and Process Events
        pangolin::FinishGlutFrame();
    }
    
    return 0;
}
