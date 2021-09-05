#include <pangolin/display/display.h>

const char* vertex_shader = R"Shader(
#version 120
attribute vec3 a_position;
varying vec2 v_pos;

void main() {
    gl_Position = vec4(a_position, 1.0);
    v_pos = a_position.xy;
}
)Shader";

const char* fragment_shader = R"Shader(
#version 120
varying vec2 v_pos;
uniform float u_time;

vec3 colorA = vec3(0.905,0.045,0.045);
vec3 colorB = vec3(0.995,0.705,0.051);

void main() {
    float pattern = sin(10*v_pos.y + u_time) * sin(10*v_pos.x + u_time) * 0.5 + 0.5;
    vec3 color = mix(colorA, colorB, pattern);
    gl_FragColor = vec4(color, 1.0);
}
)Shader";

// Here is an example of Pangolin being used *just* for windowing.
// Influenced by https://learnopengl.com/Getting-started/Hello-Triangle
void sample()
{
    pangolin::CreateWindowAndBind("Classic GL Triangle With VBO and Shader", 500, 500);

    // Build and compile our shader program
    // ------------------------------------

    // Create a vertex shader GL object
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);

    // Tell GL the source code to use
    glShaderSource(vertexShader, 1, &vertex_shader, NULL);

    // Actually compile the program
    glCompileShader(vertexShader);

    // Check if the compilation was successfull, and print anyn errors
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // Repeat for the Fragment shader
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragment_shader, NULL);
    glCompileShader(fragmentShader);
    // check for shader compile errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // Link the vertex and fragment shaders into one complete program
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // Check for linking errors
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }

    // Delete the now unused shader objects
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Setup the Triangle VBO
    // ------------------------------------

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

    // Enable use of the shader that we created for future rendering commands
    glUseProgram(shaderProgram);

    glClearColor(0.64f, 0.5f, 0.81f, 0.0f);

    // Setup a variable to progress a simple animation as a function of time
    float time = 0.01f;

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

        GLint u_time = glGetUniformLocation(shaderProgram, "u_time");
        glUniform1f( u_time, time);

        // Connect the first 3 of these vertices to form a triangle!
        glDrawArrays(GL_TRIANGLES, 0, 3);

        // Disable the stuff we enabled...
        glDisableClientState(GL_VERTEX_ARRAY);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        time += 0.01;

        pangolin::FinishFrame();
    }

    // Deallocate the OpenGL buffer we made
    glDeleteBuffers(1, &VBO);

    // Deallocate the GlSl Shader program
    glDeleteProgram(shaderProgram);
}

int main( int /*argc*/, char** /*argv*/ )
{
    sample();
    return 0;
}
