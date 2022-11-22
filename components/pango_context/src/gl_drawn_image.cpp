#include <pangolin/gui/drawn_image.h>

#include <pangolin/context/factory.h>
#include <pangolin/render/gl_vao.h>
#include <pangolin/gl/glsl_program.h>
#include <pangolin/gl/uniform.h>

#include "camera_utils.h"

namespace pangolin
{

struct GlDrawnImage : public DrawnImage
{
    // This program renders an image on the z=0 plane in a discrete pixel
    // convention such that x=0,y=0 is the center of the top-left pixel.
    // As such the vertex extremes for the texture map from (-0.5,-0.5) to
    // (w-0.5,h-0.5).
    // TODO: Add a flag to take convention as configuration.

    void draw( const DrawLayer::ViewParams& view, const Eigen::Vector2i& /* viewport_dim */ ) override {
        // ensure we're synced
        image->sync();

        PANGO_GL(glActiveTexture(GL_TEXTURE0));
        auto bind_im = image->bind();
        auto bind_prog = prog->bind();
        auto bind_vao = vao.bind();

        u_image_size = toEigen(image->imageSize()).cast<float>();
        u_intrinsics = linearClipFromCamera(view.camera, view.near_far).cast<float>();
        u_cam_from_world = view.camera_from_world.cast<float>().matrix();

        // TODO: load from DrawnImage param if specified.
        if(image->pixelType().num_channels == 1 && u_colormap_index.getValue() == Palette::none) {
            u_color_transform = (Eigen::Matrix4f() << 1,0,0,0,  1,0,0,0,  1,0,0,0, 0,0,0,1).finished();
        }else{
            u_color_transform = Eigen::Matrix4f::Identity();
        }

        u_colormap_index = colormap;

        PANGO_GL(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));
    }

    MinMax<Eigen::Vector3d> boundsInParent() const override {
        return {
            Eigen::Vector3d(-0.5, -0.5, 1.0),
            Eigen::Vector3d( image->imageSize().width - 0.5, image->imageSize().height - 0.5, 1.0)
        };
    }
private:
    const Shared<GlSlProgram> prog = GlSlProgram::Create({
        .sources = {{ .origin="/components/pango_opengl/shaders/main_image.glsl" }}
    });
    GlVertexArrayObject vao = {};
    const GlUniform<int> texture_unit = {"image"};
    const GlUniform<Eigen::Matrix4f> u_intrinsics = {"proj"};
    const GlUniform<Eigen::Matrix4f> u_cam_from_world = {"cam_from_world"};
    const GlUniform<Eigen::Vector2f> u_image_size = {"image_size"};
    const GlUniform<Eigen::Matrix4f> u_color_transform = {"color_transform"};
    const GlUniform<Palette> u_colormap_index = {"colormap_index"};
};

PANGO_CREATE(DrawnImage) {
    auto r = Shared<GlDrawnImage>::make();
    r->colormap = p.colormap;
    r->interpolation = p.interpolation;
    r->color_transform = p.color_transform;
    if(!p.image.isEmpty()) {
        r->image->update(p.image);
    }
    return r;
}

}
