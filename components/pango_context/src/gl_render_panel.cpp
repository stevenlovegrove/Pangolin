#include <pangolin/gui/render_panel.h>
#include <pangolin/context/factory.h>
#include <pangolin/handler/handler.h>

namespace pangolin
{

struct RenderPanelImpl : public RenderPanel {
    RenderPanelImpl(const RenderPanelImpl::Params& p) {}

    void renderIntoRegion(const RenderParams&) override {

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
};

PANGO_CREATE(RenderPanel) {
    return Shared<RenderPanelImpl>::make(p);
}

}
