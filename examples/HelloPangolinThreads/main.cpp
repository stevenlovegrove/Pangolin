#include <pangolin/display/display.h>
#include <pangolin/display/view.h>
#include <pangolin/display/pangolin_gl.h>
#include <pangolin/handler/handler.h>
#include <pangolin/gl/gl.h>
#include <pangolin/gl/gldraw.h>
#include <thread>

std::mutex lock;
void setup(std::string &window_name) {
    // create a window and bind its context to the main thread
    
    // enable depth
    //glEnable(GL_DEPTH_TEST);
    // unset the current context from the main thread
    //pangolin::GetBoundWindow()->RemoveCurrent();
}

void run(std::string &window_name) {
    // fetch the context and bind it to this thread
    pangolin::CreateWindowAndBind(window_name, 640, 480);
    auto context = pangolin::GetCurrentContext();
    // we manually need to restore the properties of the context
    glEnable(GL_DEPTH_TEST);

    // Define Projection and initial ModelView matrix
   pangolin::OpenGlRenderState s_cam(
        pangolin::ProjectionMatrix(640,480,420,420,320,240,0.2,100),
        pangolin::ModelViewLookAt(-2,2,-2, 0,0,0, pangolin::AxisY)
    );
   pangolin::RegisterKeyPressCallback('0', [&]()
                                       { std::cout <<window_name <<std::endl; });
    // Create Interactive View in window
    pangolin::Handler3D handler(s_cam);
    pangolin::View& d_cam = pangolin::CreateDisplay()
            .SetBounds(0.0, 1.0, 0.0, 1.0, -640.0f/480.0f)
            .SetHandler(&handler);
    while( !pangolin::ShouldQuit() )
    {
        
        // Clear screen and activate view to render into
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        d_cam.Activate(s_cam);

        // Render OpenGL Cube
        pangolin::glDrawColouredCube();

        // Swap frames and Process Events
        pangolin::FinishFrame();
    }

    // unset the current context from the main thread
    pangolin::GetBoundWindow()->RemoveCurrent();
}

int main( int /*argc*/, char** /*argv*/ )
{
    
    std::string window_name = "HelloPangolinThreads";
    std::string window_name2 = "HelloPangolinThreads2";
    // create window and context in the main thread
    //setup(window_name);
        std::thread render_loop;

    
    //setup(window_name2);
    // use the context in a separate rendering thread
    std::thread render_loop2;
    
    render_loop2 = std::thread(run,std::ref(window_name2));
   render_loop = std::thread(run,std::ref(window_name));
     render_loop.join();

    return 0;
}
