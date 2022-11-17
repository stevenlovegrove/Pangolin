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
    Eigen::Array3f debug_random_color;

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
        objects_(p.objects)
    {
        debug_random_color = (Eigen::Array3f::Random() + 1.0f) / 2.0;
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

    void renderIntoRegion(const Context& c, const RenderParams& p) override {
        ScopedGlEnable en_scissor(GL_SCISSOR_TEST);
        c.setViewport(p.region);

        for(auto& obj : objects_) {
            if(DrawnImage* im = dynamic_cast<DrawnImage*>(obj.ptr())) {
                if( (!camera_ || !camera_from_world_) && !setDefaultImageParams(*im)) continue;
                PANGO_ENSURE(camera_ && camera_from_world_);
                render_image_.draw(*im, *camera_, *camera_from_world_, near_far_);
            }else if(DrawnPrimitives* prim = dynamic_cast<DrawnPrimitives*>(obj.ptr())) {
                if(!camera_ || !camera_from_world_) setDefaultPrimitivesParams(*prim);
                PANGO_ENSURE(camera_ && camera_from_world_);
                render_primitives_.draw(*prim, *camera_, *camera_from_world_, near_far_);
            }else if(DrawnSolids* solids = dynamic_cast<DrawnSolids*>(obj.ptr())) {
                if(!camera_ || !camera_from_world_) setDefaultPrimitivesParams(*prim);
                PANGO_ENSURE(camera_ && camera_from_world_);
                render_solids_.draw(*solids, *camera_, *camera_from_world_, near_far_);
            }
        }
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
    DrawnImageProgram render_image_;
    DrawnPrimitivesProgram render_primitives_;
    DrawnSolidsProgram render_solids_;
};

PANGO_CREATE(DrawLayer) {
    return Shared<DrawLayerImpl>::make(p);
}

}
