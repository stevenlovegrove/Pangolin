#include <pangolin/display/display.h>

// Here is an example of Pangolin being used *just* for windowing.
// We're using
void sample()
{
    pangolin::CreateWindowAndBind("Classic GL Triangle with VBO", 500, 500);

    // List coordinates of a triangle
    const float vertices[] = {
        -0.5f, -0.5f, 0.0f,
         0.5f, -0.5f, 0.0f,
         0.0f,  0.5f, 0.0f
    };

    // Create an OpenGL Buffer to store these coordinates
    unsigned int VBO;
    glGenBuffers(1, &VBO);

    // Set that buffer as the current GL buffer
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    // Copy our host data into the currently bound OpenGL buffer
    // GL_STATIC_DRAW is a hint about how we'll use the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glClearColor(0.64f, 0.5f, 0.81f, 0.0f);
    glColor3f(0.29f, 0.71f, 1.0f);

    while( !pangolin::ShouldQuit() )
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Set our buffer as the current one for OpenGL
        glBindBuffer(GL_ARRAY_BUFFER, VBO);

        // This buffer contains floating point vertices with 3 dimensions.
        // They starts from the 0th element and are packed without padding.
        glVertexPointer(3, GL_FLOAT, 0, 0);

        // Use Them!
        glEnableClientState(GL_VERTEX_ARRAY);

        // Connect the first 3 of these vertices to form a triangle!
        glDrawArrays(GL_TRIANGLES, 0, 3);

        // Disable the stuff we enabled...
        glDisableClientState(GL_VERTEX_ARRAY);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        pangolin::FinishFrame();
    }

    // Deallocate the OpenGL buffer we made
    glDeleteBuffers(1, &VBO);
}

int main( int /*argc*/, char** /*argv*/ )
{
    sample();
    return 0;
}
