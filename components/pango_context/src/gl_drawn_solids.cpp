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
    void draw( const DrawLayer::ViewParams& params) override {
        auto bind_prog = prog->bind();
        auto bind_vao = vao.bind();

        u_cam_from_clip = (params.clip_from_image * params.image_from_camera).inverse().cast<float>();
        u_world_from_cam = params.camera_from_world.inverse().cast<float>().matrix();
        u_znear_zfar = Eigen::Vector2f(params.near_far.min(), params.near_far.max());

        std::optional<ScopedBind<DeviceTexture>> bind_unprojmap;
        if(params.unproject_map && !params.unproject_map->empty()) {
            PANGO_GL(glActiveTexture(GL_TEXTURE0));
            bind_unprojmap = params.unproject_map->bind();
            u_use_unproject_map = true;
        }

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
    const GlUniform<Eigen::Matrix4f> u_cam_from_clip = {"camera_from_clip"};
    const GlUniform<Eigen::Matrix4f> u_world_from_cam = {"world_from_cam"};
    const GlUniform<Eigen::Vector2f> u_znear_zfar = {"znear_zfar"};
    const GlUniform<bool> u_use_unproject_map = {"use_unproject_map"};
};

PANGO_CREATE(DrawnSolids) {
    auto r = Shared<GlDrawnSolids>::make();
    r->parent_from_drawable = p.parent_from_drawable;
    return r;
}

}
