#include <pangolin/pangolin.h>
#include <thread>

void SomeGui(const std::string& gui_name, const pangolin::Colour& background_color, const bool on_exit_close_all)
{
    pangolin::CreateWindowAndBind(gui_name,640,480);
    glEnable(GL_DEPTH_TEST);

    pangolin::OpenGlRenderState s_cam(
        pangolin::ProjectionMatrix(640,480,420,420,320,240,0.2,100),
        pangolin::ModelViewLookAt(-2,2,-2, 0,0,0, pangolin::AxisY)
    );

    pangolin::Handler3D handler(s_cam);
    pangolin::View& d_cam = pangolin::CreateDisplay()
            .SetBounds(0.0, 1.0, 0.0, 1.0, -640.0f/480.0f)
            .SetHandler(&handler);

    glClearColor(background_color.r, background_color.g, background_color.b, background_color.a);

    while( !pangolin::ShouldQuit() )
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        d_cam.Activate(s_cam);
        pangolin::glDrawColouredCube();
        pangolin::FinishFrame();
    }

    pangolin::DestroyWindow(gui_name);

    if(on_exit_close_all) {
        pangolin::QuitAll();
    }
}


int main( int /*argc*/, char** /*argv*/ )
{
    // A colour map to help visualize different windows
    pangolin::ColourWheel w;

    // Open a couple of windows Sequentially
    SomeGui("FirstWindow", w.GetUniqueColour(), false);
    SomeGui("SecondWindow", w.GetUniqueColour(), false);

    // Then open a few windows simultaneously
    constexpr size_t num_windows = 2;
    std::thread threads[num_windows];
    for(size_t i=0; i < num_windows; ++i) {
        threads[i] = std::thread(&SomeGui, pangolin::FormatString("parallel %", i), w.GetUniqueColour(), false);
    }
    SomeGui("Master Window. Closing this will close all!", w.GetUniqueColour(), true);

    // Wait for windows to get closed
    for(size_t i=0; i < num_windows; ++i) {
        threads[i].join();
    }
}
