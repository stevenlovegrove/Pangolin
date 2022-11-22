#include <pangolin/gui/drawn_checker.h>

#include <pangolin/context/factory.h>
#include <pangolin/render/gl_vao.h>
#include <pangolin/gl/glsl_program.h>
#include <pangolin/gl/uniform.h>

#include "camera_utils.h"

namespace pangolin
{

struct GlDrawnChecker : public DrawnChecker
{
    GlDrawnChecker(const DrawnChecker::Params& p)
    {
        auto bind_prog = prog->bind();
        u_color1 = p.check_color_1;
        u_color2 = p.check_color_2;
        u_checksize = p.check_size_pixels;
    }

    void draw( const DrawLayer::ViewParams& view, const Eigen::Vector2i& viewport_dim ) override {
        auto bind_prog = prog->bind();
        auto bind_vao = vao.bind();

        u_viewport_size = viewport_dim.cast<float>();
        PANGO_GL(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));
    }

    MinMax<Eigen::Vector3d> boundsInParent() const override {
        return MinMax<Eigen::Vector3d>::closed();
    }
private:
    const Shared<GlSlProgram> prog = GlSlProgram::Create({
        .sources = {{ .origin="/components/pango_opengl/shaders/main_checker.glsl" }}
    });
    GlVertexArrayObject vao = {};
    const GlUniform<Eigen::Vector2f> u_viewport_size = {"viewport_size"};
    const GlUniform<Eigen::Vector4f> u_color1 = {"color1"};
    const GlUniform<Eigen::Vector4f> u_color2 = {"color2"};
    const GlUniform<int> u_checksize = {"checksize"};
};

PANGO_CREATE(DrawnChecker) {
    return Shared<GlDrawnChecker>::make(p);
}

}
