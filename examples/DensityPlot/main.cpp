#include <pangolin/pangolin.h>
#include <pangolin/gl/glvbo.h>
#include <pangolin/gl/glsl.h>
#include <random>

#include <Eigen/Eigen>

const char* shader_vert = R"glsl(
    #version 150 core
    in vec3 pos;
    uniform mat4 T_co;

    void main()
    {
        gl_Position = T_co * vec4(pos, 1.0);
    }
)glsl";

const char* shader_geom = R"glsl(
    #version 150 core

    layout(points) in;
    layout(triangle_strip, max_vertices = 4) out;
    uniform float rad;
    uniform mat4 Proj;
    out vec2 uv;

    void main()
    {
        gl_Position = Proj * (gl_in[0].gl_Position + vec4(-rad, -rad, 0.0, 0.0));
        uv = vec2(-1,-1);
        EmitVertex();
        gl_Position = Proj * (gl_in[0].gl_Position + vec4(-rad, +rad, 0.0, 0.0));
        uv = vec2(-1,+1);
        EmitVertex();
        gl_Position = Proj * (gl_in[0].gl_Position + vec4(+rad, -rad, 0.0, 0.0));
        uv = vec2(+1,-1);
        EmitVertex();
        gl_Position = Proj * (gl_in[0].gl_Position + vec4(+rad, +rad, 0.0, 0.0));
        uv = vec2(+1,+1);
        EmitVertex();
        EndPrimitive();
    }
)glsl";

const char* shader_frag = R"glsl(
    #version 150 core
    in  vec2 uv;
    out vec4 outColor;
    uniform float weight;

    void main()
    {
        float sigma = 1.0/2.5;
        float sigma_sq = sigma*sigma;
        float dist_sq = uv[0]*uv[0] + uv[1]*uv[1];
        float prob = 1.0 - weight*exp(-dist_sq / (2*sigma_sq));
        outColor = vec4(1.0, 1.0, 1.0, prob);
    }
)glsl";


int main( int /*argc*/, char** /*argv*/ )
{
    using namespace pangolin;

    pangolin::CreateWindowAndBind("Main",640,480);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_ALWAYS);
    glEnable( GL_BLEND );
    glBlendFunc(GL_ZERO, GL_SRC_ALPHA);
    glClearColor(1.0,1.0,1.0,1.0);

    // Define Projection and initial ModelView matrix
    pangolin::OpenGlRenderState s_cam(
        pangolin::ProjectionMatrix(640,480,420,420,320,240,0.2,100),
        pangolin::ModelViewLookAt(-2,2,-2, 0,0,0, pangolin::AxisY)
    );

    // Create Interactive View in window
    pangolin::Handler3D handler(s_cam);
    pangolin::View& d_cam = pangolin::CreateDisplay()
            .SetBounds(0.0, 1.0, 0.0, 1.0, -640.0f/480.0f)
            .SetHandler(&handler);

    std::random_device rd{};
    std::mt19937 gen{rd()};

    Eigen::Matrix<Eigen::Vector3f,Eigen::Dynamic,Eigen::Dynamic> data(1000,1000);
    std::normal_distribution<> d{0.0,1.0};
    for(int r=0; r < data.rows(); ++r) {
        for(int c=0; c < data.cols(); ++c) {
//            const float x = d(gen);
//            const float y = d(gen);
//            const float z = d(gen);
//            data(r,c) = Eigen::Vector3f(x,y,z);

            const float x = 6*M_PI * r / (float)data.rows();
            const float y = 6*M_PI * c / (float)data.cols();
            data(r,c) = Eigen::Vector3f(x,y,sin(x)+cos(y));
        }
    }

    GlBuffer vbo(GlArrayBuffer, data.rows()*data.cols(), GL_FLOAT, 3, GL_STATIC_DRAW);
    vbo.Upload(data.data(), data.rows()*data.cols() * sizeof(Eigen::Vector3f));

    GlSlProgram prog;
    prog.AddShader(GlSlVertexShader, shader_vert);
    prog.AddShader(GlSlGeometryShader, shader_geom);
    prog.AddShader(GlSlFragmentShader, shader_frag);
    prog.Link();
//    prog.Bind();

    // Specify layout of point data

    pangolin::Var<float> rad("geom.rad", 0.01);
    pangolin::Var<float> weight("geom.weight", 0.1);

    while( !pangolin::ShouldQuit() )
    {
        // Clear screen and activate view to render into
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        d_cam.Activate(s_cam);

        {
            prog.Bind();
            prog.SetUniform("rad", rad.Get());
            prog.SetUniform("weight", weight.Get());
            prog.SetUniform("T_co", s_cam.GetModelViewMatrix() );
            prog.SetUniform("Proj", s_cam.GetProjectionMatrix() );
            GLint posAttrib = prog.GetAttributeHandle("pos");
            glEnableVertexAttribArray(posAttrib);
            vbo.Bind();
            glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 0, 0);
            RenderVbo(vbo, GL_POINTS);
            vbo.Unbind();
            glDisableVertexAttribArray(posAttrib);
            prog.Unbind();
        }

        // Swap frames and Process Events
        pangolin::FinishFrame();
    }

    return 0;
}
