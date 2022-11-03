#include <pangolin/gui/draw_layer.h>
#include <pangolin/context/factory.h>
#include <pangolin/handler/handler.h>
#include <pangolin/gl/glsl_program.h>
#include <pangolin/gl/uniform.h>
#include <pangolin/gl/gl.h>
#include <pangolin/gl/glvao.h>

#include "glutils.h"

#include <unordered_map>

namespace pangolin
{

struct DrawnImageProgram
{
    void draw(const DrawnImage& drawn_image)
    {
        PANGO_GL(glActiveTexture(GL_TEXTURE0));
        auto bind_im = drawn_image.image->bind();
        auto bind_prog = prog->bind();
        auto bind_vao = vao.bind();
        PANGO_GL(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));
    }
private:
    const Shared<GlSlProgram> prog = GlSlProgram::Create({
        .sources = {{ .origin="/components/pango_opengl/shaders/main_image.glsl" }}
    });
    GlVertexArrayObject vao = {};
    const GlUniform<int> texture_unit = {"image", 0};
};

struct DrawLayerImpl : public DrawLayer {
    Eigen::Array3f debug_random_color;

    DrawLayerImpl(const DrawLayerImpl::Params& p)
        : size_hint_(p.size_hint),
        handler_(p.handler),
        cam_from_world_(p.cam_from_world),
        intrinsic_k_(p.intrinsic_k),
        non_linear_(p.non_linear),
        objects_(p.objects)
    {
        debug_random_color = (Eigen::Array3f::Random() + 1.0f) / 2.0;
    }

    void renderIntoRegion(const RenderParams& p) override {
        ScopedGlEnable en_scissor(GL_SCISSOR_TEST);
        setGlViewport(p.region);
        setGlScissor(p.region);
        glClearColor(debug_random_color[0], debug_random_color[2], debug_random_color[2], 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        for(auto& obj : objects_) {
            if(DrawnImage* im = dynamic_cast<DrawnImage*>(obj.ptr())) {
                render_image.draw(*im);
            }
        }
    }

    bool handleEvent(const Event&) override {
        return false;
    }

    Size sizeHint() const override {
        return size_hint_;
    }

    void setProjection(
        const Eigen::Matrix4d& intrinsic_k,
        const NonLinearMethod non_linear = {},
        double duration_seconds = 0.0) override {

    }

    void setCamFromWorld(
        const Eigen::Matrix4d& cam_from_world,
        double duration_seconds = 0.0) override {

    }

    void setHandler(const std::shared_ptr<Handler>& handler) override {
        handler_ = handler;
    }

    MinMax<Eigen::Vector3d> getSceneBoundsInWorld() const override {
        return bounds_;
    }

    void add(const Shared<Drawable>& r) override {
        objects_.push_back(r);
    }

    void remove(const Shared<Drawable>& r) override {
        throw std::runtime_error("Not implemented yet...");
    }

    void clear() override {
        objects_.clear();
    }

    Size size_hint_;
    MinMax<Eigen::Vector3d> bounds_;
    std::shared_ptr<Handler> handler_;

    Eigen::Matrix4d cam_from_world_;
    Eigen::Matrix4d intrinsic_k_;
    NonLinearMethod non_linear_;

    std::vector<Shared<Drawable>> objects_;
    DrawnImageProgram render_image;
};

PANGO_CREATE(DrawLayer) {
    return Shared<DrawLayerImpl>::make(p);
}

}
