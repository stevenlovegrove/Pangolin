#include <pangolin/pangolin.h>
#include <pangolin/geometry/geometry.h>
#include <pangolin/gl/glsl.h>
#include <pangolin/gl/glvbo.h>

#include <pangolin/utils/file_utils.h>

#include <pangolin/geometry/geometry_ply.h>
#include <pangolin/geometry/glgeometry.h>

#include <pangolin/utils/argagg.hpp>

#include "shader.h"

inline std::vector<std::string> ExpandGlobOption(const argagg::option_results& opt)
{
    std::vector<std::string> expanded;
    for(const auto& o : opt.all)
    {
        const std::string r = o.as<std::string>();
        pangolin::FilesMatchingWildcard(r, expanded);
    }
    return expanded;
}

template<typename Tout, typename Tin, typename F>
inline std::vector<Tout> TryLoad(const std::vector<Tin>& in, const F& load_func)
{
    std::vector<Tout> loaded;
    for(const Tin& file : in)
    {
        try {
            loaded.emplace_back(load_func(file));
        }catch(std::exception) {
        }
    }
    return loaded;
}

int main( int argc, char** argv )
{
    using namespace pangolin;

    argagg::parser argparser {{
        { "help", {"-h", "--help"}, "Print usage information and exit.", 0},
        { "model", {"-m","--model"}, "3D Model to load (obj or ply)", 1},
        { "matcap", {"--matcap"}, "Matcap (material capture) images to load for shading", 1},
        { "mode", {"--mode"}, "Render mode to use {show_uv, show_texture, show_color, show_normal, show_matcap, show_vertex}", 1},
        { "worldnormals", {"--world","-w"}, "Use world normals instead of camera normals", 0},
        { "bounds", {"--aabb"}, "Show axis-aligned bounding-box", 0},
    }};

    argagg::parser_results args = argparser.parse(argc, argv);
    if ( (bool)args["help"] || !args.has_option("model")) {
        std::cerr << "usage: ModelViewer [options]" << std::endl
                  << argparser << std::endl;
        return 0;
    }

    // Options
    enum class RenderMode { uv=0, tex, color, normal, matcap, vertex, num_modes };
    const std::string mode_names[] = {"SHOW_UV", "SHOW_TEXTURE", "SHOW_COLOR", "SHOW_NORMAL", "SHOW_MATCAP", "SHOW_VERTEX"};
    const char mode_key[] = {'u','t','c','n','m','v'};
    RenderMode current_mode = RenderMode::normal;
    for(int i=0; i < (int)RenderMode::num_modes; ++i)
        if(pangolin::ToUpperCopy(args["mode"].as<std::string>("SHOW_NORMAL")) == mode_names[i])
            current_mode = RenderMode(i);
    bool world_normals = args.has_option("worldnormals");
    bool show_bounds = args.has_option("bounds");
    int mesh_to_show = -1;

    // Create Window for rendering
    pangolin::CreateWindowAndBind("Main",640,480);
    glEnable(GL_DEPTH_TEST);

    // Load Geometry
    using GeomBB = std::pair<GlGeometry, Eigen::AlignedBox3f>;
    Eigen::Vector3f center(0,0,0);
    Eigen::Vector3f view(0,0,0);
    std::vector<GeomBB> glgeoms = TryLoad<GeomBB>(ExpandGlobOption(args["model"]), [&center,&view](const std::string& f){
        const pangolin::Geometry g = pangolin::LoadGeometry(f);
        const auto aabb = pangolin::GetAxisAlignedBox(g);
        center += aabb.center();
        view += center + Eigen::Vector3f(1.2,1.2,1.2) * std::max( (aabb.max() - center).norm(), (center - aabb.min()).norm());
        return GeomBB(pangolin::ToGlGeometry(g), aabb);
    });
    center.array() /= (float)glgeoms.size();
    view.array() /= (float)glgeoms.size();

    // Load Any matcap materials
    size_t matcap_index = 0;
    std::vector<pangolin::GlTexture> matcaps = TryLoad<pangolin::GlTexture>(ExpandGlobOption(args["matcap"]), [](const std::string& f){
        return pangolin::GlTexture(pangolin::LoadImage(f));
    });

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

    // GlSl Graphics shader program for display
    pangolin::GlSlProgram prog;
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
    LoadProgram(current_mode);

    // Setup keyboard shortcuts.
    for(int i=0; i < (int)RenderMode::num_modes; ++i)
        pangolin::RegisterKeyPressCallback(mode_key[i], [&,i](){LoadProgram((RenderMode)i);});
    pangolin::RegisterKeyPressCallback('=', [&](){matcap_index = (matcap_index+1)%matcaps.size();});
    pangolin::RegisterKeyPressCallback('-', [&](){matcap_index = (matcap_index+matcaps.size()-1)%matcaps.size();});
    pangolin::RegisterKeyPressCallback('w', [&](){world_normals = !world_normals;});
    pangolin::RegisterKeyPressCallback('b', [&](){show_bounds = !show_bounds;});
    pangolin::RegisterKeyPressCallback(PANGO_SPECIAL + PANGO_KEY_RIGHT, [&](){mesh_to_show = (mesh_to_show + 1) % glgeoms.size();});
    pangolin::RegisterKeyPressCallback(PANGO_SPECIAL + PANGO_KEY_LEFT, [&](){mesh_to_show = (mesh_to_show + glgeoms.size()-1) % glgeoms.size();});
    pangolin::RegisterKeyPressCallback(' ', [&](){mesh_to_show = -1;});

    while( !pangolin::ShouldQuit() )
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if(d_cam.IsShown()) {
            d_cam.Activate(s_cam);

            if(show_bounds) {
                for(size_t i=0; i < glgeoms.size(); ++i)
                {
                    if(mesh_to_show==-1 || mesh_to_show == (int)i) {
                        glColor3f(1.0,0.0,0.0);
                        pangolin::glDrawAlignedBox(glgeoms[i].second);
                    }
                }
            }

            prog.Bind();
            prog.SetUniform("KT_cw", s_cam.GetProjectionModelViewMatrix() );
            prog.SetUniform("T_cam_norm", world_normals ? pangolin::IdentityMatrix() : s_cam.GetModelViewMatrix() );
            for(size_t i=0; i < glgeoms.size(); ++i)
            {
                if(mesh_to_show==-1 || mesh_to_show == (int)i) {
                    pangolin::GlDraw(
                        prog, glgeoms[i].first, (current_mode == RenderMode::matcap && matcap_index < matcaps.size())
                                ? &(matcaps[matcap_index]) : nullptr
                    );
                }
            }
            prog.Unbind();
        }

        pangolin::FinishFrame();
    }

    return 0;
}
