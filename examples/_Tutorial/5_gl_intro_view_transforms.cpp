#include <pangolin/display/display.h>
#include <pangolin/display/view.h>
#include <pangolin/gl/opengl_render_state.h>
#include <pangolin/gl/gldraw.h>

// Please read the following fantastic resource on the
// OpenGL Transformation pipeline:
// http://www.songho.ca/opengl/gl_transform.html
//
// In order to manipulate the position of objects within a scene and the
// position and projection action of a virtual OpenGL camera, we must
// understand the distinct set of coordinate systems that OpenGL uses.
// The Terminology for OpenGL, DirectX, Computer Vision and Robotics Literature
// are all a bit different but the concepts remain the same.

void sample()
{
    // Define the core parameters of a virtual OpenGL Camera which we will use
    // to show a simple scene. These quantities are best thought of as analogous
    // to the backplane dimensions and focal length (all in the same unit, say mm)
    // of a virtual pinhole camera (i.e. a camera with no lens and just a pinhole
    // apperture).
    const int gl_camera_width = 100;
    const int gl_camera_height = 100;
    const double gl_camera_focal_length = 100.0;

    // Create an application window of arbitrary dimensions
    pangolin::CreateWindowAndBind("Pango GL Triangle with VBO", 1024, 768);

    // Create a child view within the parent window which will stretch and grow
    // with the application window but maintain a fixed aspect of width to height
    // that will match the virtual camera we wish to render with respect to.
    auto& view = pangolin::CreateDisplay()
                    .SetAspect( (float)gl_camera_width / (float)gl_camera_height);

    pangolin::OpenGlRenderState render_transforms(
        // The OpenGL Projection Matrix handles the transform from
        // Camera coordinates into Normalized Device Coordinates (within the Viewport)
        // It can be used to specify the action of the virtual OpenGL Camera
        pangolin::ProjectionMatrix(
            gl_camera_width, gl_camera_height,
            gl_camera_focal_length, gl_camera_focal_length,
            gl_camera_width/2.0, gl_camera_height/2.0,
            0.01,  // 'Near clipping plane' - The closest and furthest thing we can render
            100.0  // 'Far clipping plane'    in scene units. Required when rasterizing
                   //                         due to finite numerical precision.
        ),
        // The OpenGL ModelView Matrix handles the transform from
        // Object coordinates into Camera Coordinates. It is useful when
        // vertices and renderable objects are specified in a fixed
        // 'object centric' coordinate system but we want to transform
        // how the object appears to move relative to the camera.
        pangolin::ModelViewLookAt(
            2.0, 2.0, 2.0, // coordinates of center of camera
            0.0, 0.0, 0.0, // coordinates to point camera towards
            pangolin::AxisY // The 'up' axis for the camera
        )
    );

    // We explicitly tell GL to only 'paint' pixels if they are in-front
    // of what is already in the framebuffer. Try turning this off and see
    // what happens!
    glEnable(GL_DEPTH_TEST);

    // Set the background color set during the glClear operation below.
    glClearColor(0.64f, 0.5f, 0.81f, 0.0f);

    for( double time=0.0; !pangolin::ShouldQuit(); time += 0.01 )
    {
        // Clear both the color buffer but also the depth buffer.
        // Try just clearing the color buffer and see what happens!
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Set the camera position as a function of time.
        // Here we use a circular orbit in XZ Plane.
        render_transforms.SetModelViewMatrix(
            pangolin::ModelViewLookAt(
                2.0*sin(time), 2.0, 2.0*cos(time),
                0.0, 0.0, 0.0,
                pangolin::AxisY
            )
        );

        if(view.IsShown()) {
            view.Activate(render_transforms);
            render_transforms.Apply();
            pangolin::glDrawColouredCube();
        }

        pangolin::FinishFrame();
    }
}

int main( int /*argc*/, char** /*argv*/ )
{
    sample();
    return 0;
}
