#include <pangolin/pangolin.h>
#include <pangolin/geometry/geometry.h>
#include <pangolin/gl/glsl.h>
#include <pangolin/gl/glvbo.h>

#include <pangolin/utils/file_utils.h>

#include <pangolin/geometry/geometry_ply.h>
#include <pangolin/geometry/glgeometry.h>

const std::string shader = R"Shader(
/////////////////////////////////////////
@start vertex
#version 120

#expect USE_UV
#expect USE_COLORS
#expect USE_NORMALS

    uniform mat4 KT_cw;
    attribute vec3 vertex;

#if USE_NORMALS
    attribute vec3 normals;
#endif

#if USE_COLORS
    attribute vec4 colors;
#endif

#if USE_UV
    attribute vec2 uv;
    varying vec2 vUV;
#endif

    varying vec4 vColor;

    void main() {
#if USE_UV
       vUV = uv;
#endif

#if USE_COLORS
       vColor = colors;
#elif USE_NORMALS
       vColor = vec4((normals + vec3(1.0,1.0,1.0)) / 2.0, 1.0);
#else
       vColor = vec4(1.0,0.0,0.0,1.0);
#endif
       gl_Position = KT_cw * vec4(vertex, 1.0);
    }

/////////////////////////////////////////
@start fragment
#version 120
#expect USE_TEXTURE
#expect USE_UV

#if USE_UV
    varying vec2 vUV;
#endif

#if USE_TEXTURE
    uniform sampler2D texture_0;
    void main() {
        gl_FragColor = texture2D(texture_0, vUV);
    }
#elif USE_UV
    void main() {
        gl_FragColor = vec4(vUV,1.0-vUV.x,1.0);
    }
#else
    varying vec4 vColor;
    void main() {
        gl_FragColor = vColor;
    }
#endif

)Shader";

int main( int argc, char** argv )
{
    if( argc < 2) {
        std::cout << "usage: ModelViewer modelname.ply" << std::endl;
        exit(-1);
    }

    const std::string model_filename = argv[1];


    pangolin::CreateWindowAndBind("Main",640,480);
    glEnable(GL_DEPTH_TEST);

    // Load Geometry
    pangolin::Geometry geom = pangolin::LoadGeometry(model_filename);
    pangolin::GlGeometry glgeom = pangolin::ToGlGeometry(geom);
    const Eigen::AlignedBox3f aabb = pangolin::GetAxisAlignedBox(geom);
    const Eigen::Vector3f center = aabb.center();
    const Eigen::Vector3f view = center + Eigen::Vector3f(1.2,1.2,1.2) * std::max( (aabb.max() - center).norm(), (center - aabb.min()).norm());

    // Define Projection and initial ModelView matrix
    pangolin::OpenGlRenderState s_cam(
        pangolin::ProjectionMatrix(640,480,420,420,320,240,0.2,1000),
        pangolin::ModelViewLookAt(view[0], view[1], view[2], center[0], center[1], center[2], pangolin::AxisY)
    );

    // Create Interactive View in window
    pangolin::Handler3D handler(s_cam);
    pangolin::View& d_cam = pangolin::CreateDisplay()
            .SetBounds(0.0, 1.0, 0.0, 1.0, -640.0f/480.0f)
            .SetHandler(&handler);


    pangolin::GlSlProgram prog;
    bool show_uv = true;
    bool show_texture = true;
    bool show_color = true;

    auto LoadProgram = [&](){
        prog.ClearShaders();
        std::map<std::string,std::string> prog_defines;
        prog_defines["USE_UV"]      = show_uv && glgeom.HasAttribute("uv")  ? "1" : "0";
        prog_defines["USE_TEXTURE"] = show_uv && show_texture && glgeom.textures.size()  ? "1" : "0";
        prog_defines["USE_COLORS"]  = show_color && glgeom.HasAttribute("colors") ? "1" : "0";
        prog_defines["USE_NORMALS"] = glgeom.HasAttribute("normals") ? "1" : "0";
        prog.AddShader(pangolin::GlSlAnnotatedShader, shader, prog_defines);
        prog.Link();
    };

    LoadProgram();

    pangolin::RegisterKeyPressCallback('t', [&](){show_texture=!show_texture; LoadProgram();});
    pangolin::RegisterKeyPressCallback('u', [&](){show_uv=!show_uv; LoadProgram();});
    pangolin::RegisterKeyPressCallback('c', [&](){show_color=!show_color; LoadProgram();});

    while( !pangolin::ShouldQuit() )
    {
        // Clear screen and activate view to render into
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        d_cam.Activate(s_cam);

        prog.Bind();
        prog.SetUniform("KT_cw", s_cam.GetProjectionModelViewMatrix() );
        pangolin::GlDraw(prog, glgeom);
        prog.Unbind();

        glColor3f(1.0,0.0,0.0);
        pangolin::glDrawAlignedBox(aabb);

        // Swap frames and Process Events
        pangolin::FinishFrame();
    }

    return 0;
}
