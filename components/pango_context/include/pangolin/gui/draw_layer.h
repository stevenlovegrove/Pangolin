#pragma once

#include <pangolin/maths/camera_look_at.h>
#include <pangolin/maths/min_max.h>
#include <pangolin/gui/layer_group.h>
#include <sophus/sensor/camera_model.h>
#include <sophus/lie/se3.h>

#include <variant>

namespace pangolin
{

template<typename T>
struct DrawableConversionTraits;

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
    virtual Shared<ViewParams> viewParams() const = 0;

    virtual void setViewConstraints(std::shared_ptr<ViewConstraints>&) = 0;
    virtual Shared<ViewConstraints> viewConstraints() const = 0;

    // Add, remove or clear Drawables from this layer
    virtual void addDrawable(const Shared<Drawable>& r) = 0;
    virtual void removeDrawable(const Shared<Drawable>& r) = 0;
    virtual void clear() = 0;

    // Convenience method to add several drawables together
    template<typename ...Ts>
    void add(const Ts&... ts) {
        (addDrawable(DrawableConversionTraits<Ts>::makeDrawable(ts)), ...);
    }

    struct Params {
        std::string name = "";
        Size size_hint = {Parts{1}, Parts{1}};
        AspectPolicy aspect_policy = AspectPolicy::mask;

        // Can leave uninitialized for best guess...
        ViewParams view_params = {};
        ViewConstraints view_constraints = {};

        // Objects to draw through modelview transform
        std::vector<Shared<Drawable>> objects = {};

        // Objects to draw in camera-frame, including 2D image-plane drawing.
        std::vector<Shared<Drawable>> objects_in_camera = {};
    };
    static Shared<DrawLayer> Create(Params p);
};

////////////////////////////////////////////////////////////////////////
// Traits for extending what types can be converted to drawables

template<DerivedFrom<DrawLayer::Drawable> L>
struct DrawableConversionTraits<Shared<L>> {
static Shared<DrawLayer::Drawable> makeDrawable(const Shared<L>& x) {
    return x;
}};

template<DerivedFrom<DrawLayer::Drawable> L>
struct DrawableConversionTraits<std::shared_ptr<L>> {
static Shared<DrawLayer::Drawable> makeDrawable(const std::shared_ptr<L>& x) {
    return x;
}};

template<typename T>
concept DrawableConvertable = requires (T x) {
    {DrawableConversionTraits<T>::makeDrawable(x)} -> SameAs<Shared<DrawLayer::Drawable>>;
};

template<DrawableConvertable T>
Shared<DrawLayer::Drawable> makeDrawable(const T& v) {
    return DrawableConversionTraits<T>::makeDrawable(v);
}

////////////////////////////////////////////////////////////////////////
// Direct conversion of anything drawable to a layer

// Helper for adding Drawables directly into layouts
template<DerivedFrom<DrawLayer::Drawable> T>
struct LayerConversionTraits<Shared<T>> {
    static Shared<Layer> makeLayer(const Shared<T>& drawable) {
        return DrawLayer::Create({ .objects = {makeDrawable(drawable)} });
    }
};

}
