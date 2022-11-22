#pragma once

#include <pangolin/maths/camera_look_at.h>
#include <pangolin/maths/min_max.h>
#include <pangolin/gui/layer_group.h>
#include <sophus/sensor/camera_model.h>
#include <sophus/lie/se3.h>

#include <variant>

namespace pangolin
{

////////////////////////////////////////////////////////////////////
/// Supports displaying interactive 2D / 3D elements
///
struct DrawLayer : public Layer
{
    struct ViewParams {
        sophus::CameraModel camera;
        sophus::Se3F64 camera_from_world;
        MinMax<double> near_far = {1e-3, 1e6};
    };
    struct ViewConstraints {
        bool image_plane_navigation = false;
        MinMax<Eigen::Vector3d> bounds_in_world = {};
    };
    struct Drawable {
        virtual ~Drawable() {}
        virtual void draw(const ViewParams&, const Eigen::Vector2i& viewport_dim) = 0;
        virtual MinMax<Eigen::Vector3d> boundsInParent() const = 0;
        Eigen::Matrix4d parent_from_drawable = Eigen::Matrix4d::Identity();
    };
    enum class AspectPolicy {
        stretch,
        crop,
        overdraw,
        mask
    };

    // Set or retrieve the shared ViewParams object for specifying the
    // intrinsic and extrinsic (sensor+lens and position) properties for
    // rendering. ViewParams can be shared across DrawLayer instances.
    virtual void setViewParams(std::shared_ptr<ViewParams>&) = 0;
    virtual Shared<ViewParams> getViewParams() const = 0;

    virtual void setViewConstraints(std::shared_ptr<ViewConstraints>&) = 0;
    virtual Shared<ViewConstraints> getViewConstraints() const = 0;

    // Add, remove or clear Drawables from this layer
    virtual void add(const Shared<Drawable>& r) = 0;
    virtual void remove(const Shared<Drawable>& r) = 0;
    virtual void clear() = 0;

    // Convenience method to add several drawables together
    template<typename T1, typename T2, typename ...Ts>
    void add(const T1& t1, const T2& t2, const Ts&... ts) {
        add(t1); add(t2);
        (add(ts), ...);
    }

    struct Params {
        std::string name = "";
        Size size_hint = {Parts{1}, Parts{1}};
        AspectPolicy aspect_policy = AspectPolicy::mask;

        // Leave as nullptr for smart initialization.
        ViewParams view_params = {};
        ViewConstraints view_constraints = {};

        // Objects to draw
        std::vector<Shared<Drawable>> objects = {};
        std::vector<Shared<Drawable>> objects_in_camera = {};
    };
    static Shared<DrawLayer> Create(Params p);
};

// Helper for adding Drawables directly into layouts
template<DerivedFrom<DrawLayer::Drawable> T>
struct LayerTraits<Shared<T>> {
    static LayerGroup toGroup(const Shared<T>& drawable) {
        return LayerTraits<Shared<Layer>>::toGroup( DrawLayer::Create({ .objects = {drawable} }));
    }
};

}
