#include <pangolin/display/display.h>
#include <pangolin/gl/gl.h>
#include <pangolin/gl/gldraw.h>

void sample()
{
    // Create a 500x500 pixel application window with an OpenGL Context
    // By default the window is double buffered if available.
    // Load any available OpenGL Extensions (through GLEW)
    pangolin::CreateWindowAndBind("Pango GL Triangle", 500, 500);

    // List coordinates of a triangle
    // These vertices will be relative to the coordinates of the window
    // which default in OpenGL to +/- 1.0 in X and Y (first two vertex ordinates)
    std::vector<Eigen::Vector3f> vertices = {
        {-0.5f, -0.5f, 0.0f},
        {0.5f, -0.5f, 0.0f },
        {0.0f,  0.5f, 0.0f }
    };

    // We want the background to be purple
    glClearColor(0.64f, 0.5f, 0.81f, 0.0f);

    // We want our triangle to be a pleasant shade of blue!
    glColor3f(0.29f, 0.71f, 1.0f);

    // We keep rendering in a loop so that the triangle will keep showing
    // and get adjusted as the window is resized. Press Escape or close the
    // window to exit the Pangolin loop.
    while( !pangolin::ShouldQuit() )
    {
        // Clear the window
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Connect the first 3  vertices to form a triangle!
        pangolin::glDrawVertices(vertices, GL_TRIANGLES);

        // Process any windowing events and swap the back and front
        // OpenGL buffers if available.
        pangolin::FinishFrame();
    }
}

int main( int /*argc*/, char** /*argv*/ )
{
    sample();
    return 0;
}
