#include <pangolin/display/display.h>
#include <pangolin/gl/glvbo.h>

void sample()
{
    pangolin::CreateWindowAndBind("Pango GL Triangle with VBO", 500, 500);

    // Create an OpenGL Buffer containing the vertices of a triangle
    pangolin::GlBuffer vbo(pangolin::GlArrayBuffer,
        std::vector<Eigen::Vector3f>{
           {-0.5f, -0.5f, 0.0f},
           { 0.5f, -0.5f, 0.0f },
           { 0.0f,  0.5f, 0.0f }
        }
    );

    glClearColor(0.64f, 0.5f, 0.81f, 0.0f);
    glColor3f(0.29f, 0.71f, 1.0f);

    while( !pangolin::ShouldQuit() )
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Connect the first 3 vertices in the GL Buffer to form a triangle!
        pangolin::RenderVbo(vbo, GL_TRIANGLES);

        pangolin::FinishFrame();
    }
}

int main( int /*argc*/, char** /*argv*/ )
{
    sample();
    return 0;
}
