#include <pangolin/gui/draw_layer.h>
#include <pangolin/context/context.h>
#include <pangolin/context/factory.h>
#include <pangolin/handler/handler.h>
#include <pangolin/render/gl_vao.h>
#include <pangolin/gl/glsl_program.h>
#include <pangolin/gl/uniform.h>
#include <pangolin/gl/gl.h>
#include <pangolin/var/var.h>

#include "camera_utils.h"
#include "gl_utils.h"
#include "gl_drawn_image.h"
#include "gl_drawn_primitives.h"
#include "gl_drawn_solids.h"

#include <unordered_map>

using namespace sophus;

namespace pangolin
{

struct DrawLayerImpl : public DrawLayer {
    DrawLayerImpl(const DrawLayerImpl::Params& p)
        : name_(p.name),
        size_hint_(p.size_hint),
        near_far_(p.near_far),
        camera_(p.camera),
        camera_from_world_(p.camera_from_world),
        handler_(p.handler),
        cam_from_world_(Eigen::Matrix4d::Identity()),
        intrinsic_k_(Eigen::Matrix4d::Identity()),
        non_linear_(),
        objects_(p.objects),
        background_(p.background)
    {
    }

    std::string name() const override
    {
        return name_;
    }

    bool setDefaultImageParams(const DrawnImage& im)
    {
        if(im.image->imageSize().isEmpty()) return false;

        const auto imsize = im.image->imageSize();

        if(!camera_) {
            auto ortho = CameraModel(imsize, CameraDistortionType::orthographic, Eigen::Vector4d{1.0,1.0, 0.0,0.0} );
            camera_ = Shared<CameraModel>::make(ortho);
        }

        if(!camera_from_world_) {
            camera_from_world_ = Shared<Se3F64>::make();
        }

        if(near_far_.empty()) {
            near_far_ = MinMax<double>(-1.0, 1.0);
        }

        if(handler_ && handler_->getCameraLimits().empty()) {
            handler_->setCameraLimits({
                {0.0, 0.0, 0.0},
                {0.0, 0.0, 0.0},
            });
            handler_->setCameraRotationLock(true);
        }

        return true;
    }

    void setDefaultPrimitivesParams(const DrawnPrimitives& /* prim */)
    {
        // TODO: based these off of the data in prim
        if(!camera_) {
            camera_ = Shared<sophus::CameraModel>::make(
                sophus::createDefaultPinholeModel({100,100})
            );
        }

        if(!camera_from_world_) {
            camera_from_world_ = Shared<sophus::Se3F64>::make(
                cameraLookatFromWorld( {0.0, 0.0, -5.0}, {0.0, 0.0, 0.0} )
            );
        }

        if(near_far_.empty()) {
            near_far_ = MinMax<double>(0.1, 1000.0);
        }
    }

    void renderObjectsIntoRegion(
        const std::vector<Shared<Drawable>>& objects,
        bool background
    ) {
        for(auto& obj : objects) {
            if(const DrawnImage* im = dynamic_cast<const DrawnImage*>(obj.ptr())) {
                if(background) {
                    const auto imsize = im->image->imageSize();
                    if(imsize.isEmpty()) continue;
                    auto ortho = CameraModel(imsize, CameraDistortionType::orthographic, Eigen::Vector4d{1.0,1.0, 0.0,0.0} );
                    render_image_.draw(*im, ortho, sophus::SE3d(), {-1.0f, 1.0f});
                }else{
                    if( (!camera_ || !camera_from_world_) && !setDefaultImageParams(*im)) continue;
                    PANGO_ENSURE(camera_ && camera_from_world_);
                    render_image_.draw(*im, *camera_, *camera_from_world_, near_far_);
                }
            }else if(const DrawnPrimitives* prim = dynamic_cast<const DrawnPrimitives*>(obj.ptr())) {
                if(!camera_ || !camera_from_world_) setDefaultPrimitivesParams(*prim);
                PANGO_ENSURE(camera_ && camera_from_world_);
                render_primitives_.draw(*prim, *camera_, *camera_from_world_, near_far_);
            }else if(const DrawnSolids* solids = dynamic_cast<const DrawnSolids*>(obj.ptr())) {
                if(!camera_ || !camera_from_world_) setDefaultPrimitivesParams(*prim);
                PANGO_ENSURE(camera_ && camera_from_world_);
                render_solids_.draw(*solids, *camera_, *camera_from_world_, near_far_);
            }
        }
    }

    void renderIntoRegion(const Context& context, const RenderParams& params) override {
        ScopedGlEnable en_scissor(GL_SCISSOR_TEST);
        context.setViewport(params.region);

        renderObjectsIntoRegion(background_, true);
        glClear(GL_DEPTH_BUFFER_BIT);
        renderObjectsIntoRegion(objects_, false);
    }

    bool handleEvent(const Context& context, const Event& event) override {
        if(handler_ && camera_from_world_ && camera_) {
            handler_->handleEvent(
                *this, *camera_from_world_,
                *camera_, near_far_,
                context, event
            );
            return true;
        }
        return false;
    }

    Size sizeHint() const override {
        return size_hint_;
    }

    void setHandler(const std::shared_ptr<Handler>& handler) override {
        handler_ = handler;
    }

    void setCamera(std::shared_ptr<sophus::CameraModel>& camera) override {
        camera_ = camera;
    }

    void setCameraPoseFromWorld( std::shared_ptr<sophus::Se3F64>& camera_from_world) override {
        camera_from_world_ = camera_from_world;
    }

    std::shared_ptr<Handler> getHandler() const override {
        return handler_;
    }
    std::shared_ptr<sophus::CameraModel> getCamera() const override {
        return camera_;
    }
    std::shared_ptr<sophus::Se3F64> getCameraPoseFromWorld() const override {
        return camera_from_world_;
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

    std::string name_;
    Size size_hint_;
    MinMax<double> near_far_;
    MinMax<Eigen::Vector3d> bounds_;

    // Used for handling and higher-level logic
    std::shared_ptr<sophus::CameraModel> camera_;
    std::shared_ptr<sophus::Se3F64> camera_from_world_;
    std::shared_ptr<Handler> handler_;

    // Actually used for rendering
    Eigen::Matrix4d cam_from_world_;
    Eigen::Matrix4d intrinsic_k_;
    NonLinearMethod non_linear_;

    std::vector<Shared<Drawable>> objects_;
    std::vector<Shared<Drawable>> background_;
    DrawnImageProgram render_image_;
    DrawnPrimitivesProgram render_primitives_;
    DrawnSolidsProgram render_solids_;
};

PANGO_CREATE(DrawLayer) {
    return Shared<DrawLayerImpl>::make(p);
}

struct ClearLayerImpl : public Layer {
    enum class Action {
        clear_depth,
        clear_color,
        clear_both
    };

    ClearLayerImpl(GLbitfield clear_mask)
    : clear_mask_(clear_mask)
    {
    }

    std::string name() const override { return "-"; }
    Size sizeHint() const override { return {Parts{0}, Parts{0}}; }

    void renderIntoRegion(const Context&, const RenderParams&) override {
        glClear(clear_mask_);
    }

    GLbitfield clear_mask_;
};


Shared<Layer> Layer::ClearZ()
{
    static auto instance = Shared<ClearLayerImpl>::make(GL_DEPTH_BUFFER_BIT);
    return instance;
}

Shared<Layer> Layer::ClearColor()
{
    static auto instance = Shared<ClearLayerImpl>::make(GL_COLOR_BUFFER_BIT);
    return instance;
}

Shared<Layer> Layer::Clear()
{
    static auto instance = Shared<ClearLayerImpl>::make(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    return instance;
}

}
