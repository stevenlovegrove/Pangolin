#include <pangolin/gui/draw_layer.h>
#include <pangolin/context/factory.h>
#include <pangolin/handler/handler.h>
#include <pangolin/gl/glplatform.h>
#include <Eigen/Core>

namespace pangolin
{

void setGlViewportAndScissor(const MinMax<Eigen::Vector2i>& bounds)
{
    const auto pos = bounds.min();
    const auto size = bounds.range();
    glViewport( pos.x(), pos.y(), size.x()+1, size.y()+1 );
    glScissor( pos.x(), pos.y(), size.x()+1, size.y()+1 );
}

// Will glEnable and glDisable GL capability regardless
// of current state.
struct [[nodiscard]] ScopedGlEnable
{
    ScopedGlEnable(GLenum cap) : cap_(cap) { glEnable(cap_); }
    ~ScopedGlEnable() { glDisable(cap_); }
    GLenum cap_;
};

struct DrawLayerImpl : public DrawLayer {
    Eigen::Array3f debug_random_color;

    DrawLayerImpl(const DrawLayerImpl::Params& p)
        : size_hint_(p.size_hint)
    {
        debug_random_color = (Eigen::Array3f::Random() + 1.0f) / 2.0;

    }

    void renderIntoRegion(const RenderParams& p) override {
        ScopedGlEnable en_scissor(GL_SCISSOR_TEST);
        setGlViewportAndScissor(p.region);
        glClearColor(debug_random_color[0], debug_random_color[2], debug_random_color[2], 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
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

    MinMax<Eigen::Vector3d> bounds_;
    std::shared_ptr<Handler> handler_;
    Size size_hint_;

    Eigen::Matrix4d intrinsic_k;
    Eigen::Matrix4d cam_from_world;
};

PANGO_CREATE(DrawLayer) {
    return Shared<DrawLayerImpl>::make(p);
}

}
