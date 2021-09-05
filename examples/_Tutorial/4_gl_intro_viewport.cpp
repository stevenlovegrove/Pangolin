#include <pangolin/display/display.h>
#include <pangolin/display/view.h>
#include <pangolin/gl/glvbo.h>

void sample()
{
    pangolin::CreateWindowAndBind("Pango GL Triangle with VBO", 1000, 500);

    // A pangolin::View is an object oriented encapsulation of a glViewport.
    // It describes which rectangle of pixels in the window map to the OpenGL
    // Normalized Device Coordinates (NDC) for rendering.
    // See https://www.songho.ca/opengl/gl_transform.html for technical description
    // of OpenGL coordinate systems and transforms.

    // The View::SetBounds() method accepts the bottom, top, left, right arguments
    // which correspond to the vertical min/max and horizontal min/max locations.
    // Units can be fractional [0,1] representing bottom/left of window to top/right.
    // You can use pangolin::Attach::Pix() to specify an absolute pixel coordinate
    // relative to the bottom or left edge instead of a fractional one.
    // pangolin::Attach::ReversePix() is instead relative to the top or right edge.
    //
    // A View is parented to the main window (pangolin::DisplayBase()) by default,
    // but that can be changed with the View::AddDisplay() method to re-child to any
    // other View. The specified bounds are relative to the parent and will
    // be recalculated automatically during resize events.

    // Create two 'views' representing the left and right half of the window.
    auto& view1 = pangolin::CreateDisplay().SetBounds(0.0, 1.0, 0.0, 0.5);
    auto& view2 = pangolin::CreateDisplay().SetBounds(0.0, 1.0, 0.5, 1.0);

    // Create an OpenGL Buffer containing the vertices of a triangle
    pangolin::GlBuffer vertices(pangolin::GlArrayBuffer,
        std::vector<Eigen::Vector3f>{
           {-0.5f, -0.5f, 0.0f},
           { 0.5f, -0.5f, 0.0f },
           { 0.0f,  0.5f, 0.0f }
        }
    );

    // Set the background color
    glClearColor(0.64f, 0.5f, 0.81f, 0.0f);

    while( !pangolin::ShouldQuit() )
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Views also maintain their expected visibility, but since we're doing our own
        // rendering inside the view we need to check this manually before drawing.
        if(view1.IsShown()) {
            // 'Activating()' a view will use the calculated absolute pixel
            // positions and pass them on to glViewport to describe where
            // to render on the screen.
            view1.Activate();

            // Render a blue triangle
            glColor3f(0.29f, 0.71f, 1.0f);
            pangolin::RenderVbo(vertices, GL_TRIANGLES);
        }

        if(view2.IsShown()) {
            view2.Activate();

            // Render a green triangle
            glColor3f(0.71f, 1.0f, 0.29f);
            pangolin::RenderVbo(vertices, GL_TRIANGLES);
        }

        // FinishFrame processes any pending windowing events such as
        // mouse clicks and resizes. Resizes will trigger View absolute
        // bounds recalculations.
        pangolin::FinishFrame();
    }
}

int main( int /*argc*/, char** /*argv*/ )
{
    sample();
    return 0;
}
