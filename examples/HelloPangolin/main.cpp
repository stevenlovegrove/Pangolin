#include <pangolin/pangolin.h>
#include <pangolin/glfont.h>
#include <pangolin/image_load.h>

int main( int argc, char** argv )
{  
    pangolin::CreateWindowAndBind("Main",640,480);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);    
    
    // Define Projection and initial ModelView matrix
    pangolin::OpenGlRenderState s_cam(
        pangolin::ProjectionMatrix(640,480,420,420,320,240,0.2,100),
        pangolin::ModelViewLookAt(-2,2,-2, 0,0,0, pangolin::AxisY)
    );
    
    // Create Interactive View in window
    pangolin::Handler3D handler(s_cam);
    pangolin::View& d_cam = pangolin::CreateDisplay()
            .SetBounds(0.0, 1.0, 0.0, 1.0, 640.0f/480.0f)
            .SetHandler(&handler);
    
    pangolin::GlFont font;
    
    pangolin::GlText test = font.Text("Testing my magnificance...");
    
    while( !pangolin::ShouldQuit() )
    {
        // Clear screen and activate view to render into
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        d_cam.Activate(s_cam);
        
        // Render OpenGL Teapot
        glColor3f(1.0,1.0,1.0);
        pangolin::glDrawColouredCube();

        glColor4f(1.0f,1.0f,1.0f,1.0f);
        test.Draw(2,2,2);
        
        pangolin::DisplayBase().ActivatePixelOrthographic();
        test.Draw(100,100);
        
        // Swap frames and Process Events
        pangolin::FinishFrame();
    }
    
    return 0;
}
