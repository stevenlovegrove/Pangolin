#include <pangolin/display/display.h>
#include <pangolin/gl/gldraw.h>
#include <pangolin/gl/glvbo.h>
#include <pangolin/gl/glsl.h>

const std::string my_shader = R"Shader(
@start vertex
#version 120
attribute vec3 a_position;
varying vec2 v_pos;

void main() {
    gl_Position = vec4(a_position, 1.0);
    v_pos = a_position.xy;
}

@start fragment
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

void sample()
{
    pangolin::CreateWindowAndBind("Pango GL Triangle With VBO and Shader", 500, 500);

    pangolin::GlBuffer vbo(pangolin::GlArrayBuffer,
        std::vector<Eigen::Vector3f>{
           {-0.5f, -0.5f, 0.0f},
           { 0.5f, -0.5f, 0.0f },
           { 0.0f,  0.5f, 0.0f }
        }
    );

    // Encapsulate a GlSl shader program to define how to render OpenGL primitives
    // The Pangolin GlSlProgram has a preprocessor which can seperate different kinds of
    // shaders and manage compile-time defines.
    pangolin::GlSlProgram prog;
    prog.AddShader( pangolin::GlSlAnnotatedShader, my_shader );
    prog.Link();
    prog.Bind();

    glClearColor(0.64f, 0.5f, 0.81f, 0.0f);

    // Setup a variable to progress a simple animation as a function of time
    float time = 0.01f;

    while( !pangolin::ShouldQuit() )
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Update the shader u_time parameter and render the triangles
        // whilst this shader is active
        prog.SetUniform("u_time", time);
        pangolin::RenderVbo(vbo, GL_TRIANGLES);
        time += 0.01;

        pangolin::FinishFrame();
    }
}

int main( int /*argc*/, char** /*argv*/ )
{
    sample();
    return 0;
}
