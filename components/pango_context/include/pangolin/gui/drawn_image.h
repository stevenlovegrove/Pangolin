#include <sophus/image/runtime_image.h>
#include <pangolin/maths/conventions.h>
#include <pangolin/render/device_texture.h>
#include <pangolin/render/colormap.h>

#include <pangolin/gui/draw_layer.h>
#include <pangolin/gui/drawn_checker.h>

namespace pangolin
{

// The 'image' frame is such that the pixels lie on the z=0 plane, with the
// image x and y axis corresponding to the world co-ordinates in -continuous-
// convention. i.e. the the x=0,y=0,z=0 frame point would be (-0.5,-0.5) in
// pixel centered integral coordinate convention.
struct DrawnImage : public DrawLayer::Drawable
{
    enum class Interpolation {
        nearest,
        bilinear,
    };

    // Image to render. Not all types will be supported by the implementation.
    Shared<DeviceTexture> image = DeviceTexture::Create();

    // How should fractional pixel coordinates be rendered (when magnified)
    Interpolation interpolation = Interpolation::nearest;

    // optional transform which maps the pixel color space to the rendered
    // output intensity
    std::optional<Eigen::MatrixXd> color_transform;

    // if a palatte beside `none` is specified, the first (red) channel is used
    // as input to the non-linear map. Colormapping occurs after the linear
    // color_transform above.
    Palette colormap = Palette::none;

    struct Params {
        sophus::IntensityImage<> image = {};
        Palette colormap = Palette::none;
        Interpolation interpolation = Interpolation::nearest;
        std::optional<Eigen::MatrixXd> color_transform;
    };
    static Shared<DrawnImage> Create(Params p);
};

////////////////////////////////////////////////////////////////////

template<typename T> concept ImageConvertable = requires (T x) {
    {sophus::IntensityImage<>(x)} -> SameAs<sophus::IntensityImage<>>;
};

template<ImageConvertable T>
sophus::CameraModel defaultOrthoCameraForImage(const T& image) {
    return sophus::CameraModel( image.imageSize(),
        sophus::CameraDistortionType::orthographic,
        Eigen::Vector4d{1.0,1.0, 0.0,0.0}
    );
}

// Helper for adding images (runtime and statically typed) directly to layouts
template<ImageConvertable T>
struct LayerConversionTraits<T> {
static Shared<Layer> makeLayer(const T& image) {
    return DrawLayer::Create({
        .objects_in_camera = {
            DrawnChecker::Create({}),
            DrawnImage::Create({.image=image})
        }
    });
}};

// Specialization of draw_layer.h's trait for DrawnImage so it defaults to
// add to .objects_in_camera instead of .objects
template<>
struct LayerConversionTraits<Shared<DrawnImage>> {
    static Shared<Layer> makeLayer(const Shared<DrawnImage>& drawable) {
        return DrawLayer::Create({
             .objects_in_camera = {DrawnChecker::Create({}), drawable}
        });
    }
};

}
