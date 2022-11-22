#include <pangolin/gui/drawn_solids.h>

#include <pangolin/context/factory.h>
#include <pangolin/render/gl_vao.h>
#include <pangolin/gl/glsl_program.h>
#include <pangolin/gl/uniform.h>

#include "camera_utils.h"

namespace pangolin
{

struct GlDrawnSolids : public DrawnSolids
{
    void draw( const DrawLayer::ViewParams& view, const Eigen::Vector2i& /* viewport_dim */ ) override {
        auto bind_prog = prog->bind();
        auto bind_vao = vao.bind();

        u_image_size = Eigen::Vector2f(view.camera.imageSize().width, view.camera.imageSize().height);
        u_kinv = linearCameraFromImage(view.camera).cast<float>();
        u_world_from_cam = view.camera_from_world.inverse().cast<float>().matrix();
        u_znear_zfar = Eigen::Vector2f(view.near_far.min(), view.near_far.max());

        PANGO_GL(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));
    }

    MinMax<Eigen::Vector3d> boundsInParent() const override {
        return MinMax<Eigen::Vector3d>();
    }

private:
    const Shared<GlSlProgram> prog = GlSlProgram::Create({
        .sources = {{ .origin="/components/pango_opengl/shaders/main_plane.glsl" }}
    });
    GlVertexArrayObject vao = {};
    const GlUniform<Eigen::Matrix3f> u_kinv = {"kinv"};
    const GlUniform<Eigen::Matrix4f> u_world_from_cam = {"world_from_cam"};
    const GlUniform<Eigen::Vector2f> u_image_size = {"image_size"};
    const GlUniform<Eigen::Vector2f> u_znear_zfar = {"znear_zfar"};
};

PANGO_CREATE(DrawnSolids) {
    auto r = Shared<GlDrawnSolids>::make();
    r->parent_from_drawable = p.parent_from_drawable;
    return r;
}

}
