#include <pangolin/pangolin.h>
#include <pangolin/geometry/geometry.h>
#include <pangolin/gl/glsl.h>
#include <pangolin/gl/glvbo.h>

#include <pangolin/utils/file_utils.h>

#include <pangolin/geometry/geometry_ply.h>
#include <pangolin/geometry/glgeometry.h>

#include <pangolin/utils/argagg.hpp>

const std::string shader = R"Shader(
/////////////////////////////////////////
@start vertex
#version 120

#expect SHOW_COLOR
#expect SHOW_NORMAL
#expect SHOW_TEXTURE
#expect SHOW_MATCAP
#expect SHOW_UV

    uniform mat4 T_cam_norm;
    uniform mat4 KT_cw;
    attribute vec3 vertex;

#if SHOW_COLOR
    attribute vec4 color;
    varying vec4 vColor;
    void main() {
        vColor = color;
#elif SHOW_NORMAL
    attribute vec3 normal;
    varying vec3 vNormal;
    void main() {
        vNormal = mat3(T_cam_norm) * normal;
#elif SHOW_TEXTURE
    attribute vec2 uv;
    varying vec2 vUV;
    void main() {
        vUV = uv;
#elif SHOW_MATCAP
    attribute vec3 normal;
    varying vec3 vNormalCam;
    void main() {
        vNormalCam = mat3(T_cam_norm) * normal;
#elif SHOW_UV
    attribute vec2 uv;
    varying vec2 vUV;
    void main() {
        vUV = uv;
#else
    varying vec3 vP;
    void main() {
        vP = vertex;
#endif
        gl_Position = KT_cw * vec4(vertex, 1.0);
    }

/////////////////////////////////////////
@start fragment
#version 120
#expect SHOW_COLOR
#expect SHOW_NORMAL
#expect SHOW_TEXTURE
#expect SHOW_MATCAP
#expect SHOW_UV

#if SHOW_COLOR
    varying vec4 vColor;
#elif SHOW_NORMAL
    varying vec3 vNormal;
#elif SHOW_TEXTURE
    varying vec2 vUV;
    uniform sampler2D texture_0;
#elif SHOW_MATCAP
    varying vec3 vNormalCam;
    uniform sampler2D matcap;
#elif SHOW_UV
    varying vec2 vUV;
#else
    varying vec3 vP;
#endif

void main() {
#if SHOW_COLOR
    gl_FragColor = vColor;
#elif SHOW_NORMAL
    gl_FragColor = vec4((vNormal + vec3(1.0,1.0,1.0)) / 2.0, 1.0);
#elif SHOW_TEXTURE
    gl_FragColor = texture2D(texture_0, vUV);
#elif SHOW_MATCAP
    vec2 uv = 0.5 * vNormalCam.xy + vec2(0.5, 0.5);
    gl_FragColor = texture2D(matcap, uv);
#elif SHOW_UV
    gl_FragColor = vec4(vUV,1.0-vUV.x,1.0);
#else
    gl_FragColor = vec4(vP / 100.0,1.0);
#endif
}
)Shader";

int main( int argc, char** argv )
{
    argagg::parser argparser {{
        { "help", {"-h", "--help"}, "Print usage information and exit.", 0},
        { "model", {"-m","--model"}, "3D Model to load (obj or ply)", 1},
        { "matcap", {"--matcap"}, "Matcap textures to load", 1},
    }};

    argagg::parser_results args = argparser.parse(argc, argv);
    if ( (bool)args["help"] || !args.has_option("model")) {
        std::cerr << "usage: ModelViewer modelname.ply" << std::endl
                  << argparser << std::endl;
        return 0;
    }

    const std::string model_filename = args["model"].as<std::string>("");
    const std::string matcaps_filename = args["matcap"].as<std::string>("");

    pangolin::CreateWindowAndBind("Main",640,480);
    glEnable(GL_DEPTH_TEST);

    // Load Geometry
    pangolin::Geometry geom = pangolin::LoadGeometry(model_filename);
    pangolin::GlGeometry glgeom = pangolin::ToGlGeometry(geom);
    const Eigen::AlignedBox3f aabb = pangolin::GetAxisAlignedBox(geom);
    const Eigen::Vector3f center = aabb.center();
    const Eigen::Vector3f view = center + Eigen::Vector3f(1.2,1.2,1.2) * std::max( (aabb.max() - center).norm(), (center - aabb.min()).norm());

    // Load Any matcap materials
    std::vector<pangolin::GlTexture> matcaps;
    size_t matcap_index = 0;
    if(!matcaps_filename.empty()) {
        std::vector<std::string> matcap_filevec;
        pangolin::FilesMatchingWildcard(matcaps_filename, matcap_filevec);
        for(const auto& f : matcap_filevec)
        {
            try {
                pangolin::GlTexture tex;
                tex.LoadFromFile(f);
                matcaps.emplace_back(std::move(tex));
            }catch(std::exception&){}
        }
        std::cout << pangolin::FormatString("Loaded % MatCap materials", matcaps.size()) << std::endl;
    }

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
    enum class RenderMode { uv=0, tex, color, normal, matcap, vertex, num_modes };
    const std::string mode_names[] = {"SHOW_UV", "SHOW_TEXTURE", "SHOW_COLOR", "SHOW_NORMAL", "SHOW_MATCAP", "OTHER"};
    const char mode_key[] = {'u','t','c','n','m','v'};
    RenderMode current_mode = RenderMode::normal;
    bool world_normals = false;


    auto LoadProgram = [&](const RenderMode mode){
        current_mode = mode;
        prog.ClearShaders();
        std::map<std::string,std::string> prog_defines;
        for(int i=0; i < (int)RenderMode::num_modes-1; ++i) {
            prog_defines[mode_names[i]] = std::to_string((int)mode == i);
        }
        prog.AddShader(pangolin::GlSlAnnotatedShader, shader, prog_defines);
        prog.Link();
    };

    LoadProgram(RenderMode::vertex);

    for(int i=0; i < (int)RenderMode::num_modes; ++i)
        pangolin::RegisterKeyPressCallback(mode_key[i], [&,i](){LoadProgram((RenderMode)i);});
    pangolin::RegisterKeyPressCallback('=', [&](){matcap_index = (matcap_index+1)%matcaps.size();});
    pangolin::RegisterKeyPressCallback('-', [&](){matcap_index = (matcap_index+matcaps.size()-1)%matcaps.size();});
    pangolin::RegisterKeyPressCallback('w', [&](){world_normals = !world_normals;});

    while( !pangolin::ShouldQuit() )
    {
        // Clear screen and activate view to render into
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        d_cam.Activate(s_cam);

        prog.Bind();
        prog.SetUniform("KT_cw", s_cam.GetProjectionModelViewMatrix() );
        prog.SetUniform("T_cam_norm", world_normals ? pangolin::IdentityMatrix() : s_cam.GetModelViewMatrix() );
        pangolin::GlDraw(
            prog, glgeom, (current_mode == RenderMode::matcap && matcap_index < matcaps.size())
                    ? &(matcaps[matcap_index]) : nullptr
        );
        prog.Unbind();

        glColor3f(1.0,0.0,0.0);
        pangolin::glDrawAlignedBox(aabb);

        // Swap frames and Process Events
        pangolin::FinishFrame();
    }

    return 0;
}
