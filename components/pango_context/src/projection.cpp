#include <pangolin/maths/projection.h>
#include <pangolin/utils/logging.h>

namespace pangolin
{

Eigen::Matrix4d transformImageFromCamera4x4(
    const Eigen::Matrix3d& K_intrinsics
) {
    Eigen::Matrix4d ret = Eigen::Matrix4d::Identity();
    ret.topLeftCorner<3,3>() = K_intrinsics;
    return ret;
}

Eigen::Matrix4d transformProjectionFromImage(
    MinMax<double> near_far_in_world_units,
    GraphicsProjection projection
) {
    const double near = near_far_in_world_units.min();
    const double far = near_far_in_world_units.max();

    PANGO_ENSURE(projection == GraphicsProjection::orthographic ||
        std::signbit(near) == std::signbit(far),
        "near and far planes must have the same sign for perspective projection");

    Eigen::Matrix4d ret = Eigen::Matrix4d::Identity();
    const bool nofar = (far == std::numeric_limits<double>::infinity());
    const double sign = std::signbit(near) ? -1.0 : +1.0;

    if(projection == GraphicsProjection::perspective) {
        ret(2,2) = nofar ? sign : (far + near) / (far - near);
        ret(2,3) = nofar ? -sign * 2.0 * near : 2.0 * far * near / (near - far);
        ret(3,3) = 0.0;
        ret(3,2) = 1.0;
    }else if(projection == GraphicsProjection::orthographic) {
        PANGO_ENSURE(!nofar, "Orthographic projection requires finite far plane");
        ret(2,2) = 2.0 / (far - near);
        ret(2,3) = -(far + near) / (far - near);
    }else{
        PANGO_UNIMPLEMENTED();
    }

    return ret;
}

Eigen::Matrix4d transformClipFromProjection(
    sophus::ImageSize size,
    ImageXy image_convention,
    ImageIndexing image_indexing
) {
    Eigen::Matrix4d ret = Eigen::Matrix4d::Identity();

    if(image_indexing == ImageIndexing::normalized_minus_one_one) {
        // Already in the right convention. Can ignore width & height.
        return ret;
    }else if(image_indexing == ImageIndexing::normalized_minus_one_one) {
        PANGO_UNIMPLEMENTED();
        return Eigen::Matrix4d::Identity();
    }else{
        const bool cont = image_indexing == ImageIndexing::pixel_continuous;
        ret(0,0) = 2.0 / size.width;
        ret(1,1) = 2.0 / size.height;
        ret(0,3) = cont ? -1.0 : 1.0 / double(size.width) - 1.0;
        ret(1,3) = cont ? -1.0 : 1.0 / double(size.height) - 1.0;

        if(image_convention == ImageXy::right_up) {
            return ret;
        }else{
            Eigen::Matrix4d flip_y = Eigen::Matrix4d::Identity();
            flip_y(1,1) = -1;
            return flip_y * ret;
        }
    }
}

Eigen::Matrix3d transformWindowFromClip(MinMax<Eigen::Array2i> viewport)
{
    // TODO: fixup for 0.5 ofsets (for discrete coords)
    const Eigen::Array2d scale = cast<double>(viewport.range()) / 2.0;
    const Eigen::Array2d offset = cast<double>(viewport.min()) + cast<double>(viewport.range()) / 2.0;
    Eigen::Matrix3d ret = Eigen::Matrix3d::Identity();
    ret(0,0) = scale[0];
    ret(1,1) = -scale[1];
    ret.topRightCorner<2,1>() = offset;
    return ret;
}

Eigen::Matrix3d projectionImageFromCamera(
    Eigen::Vector2d focal_distance_pixels,
    Eigen::Vector2d principle_point
){
    return (Eigen::Matrix3d() <<
        focal_distance_pixels[0], 0.0, principle_point[0],
        0.0, focal_distance_pixels[1], principle_point[1],
        0.0, 0.0, 1.0).finished();
}

Eigen::Matrix3d invProjectionCameraFromImage(
    Eigen::Vector2d focal_distance_pixels,
    Eigen::Vector2d principle_point
){
    return (Eigen::Matrix3d() <<
        1.0 / focal_distance_pixels[0], 0.0, -principle_point[0] / focal_distance_pixels[0],
        0.0, 1.0 / focal_distance_pixels[1], -principle_point[1] / focal_distance_pixels[1],
        0.0, 0.0, 1.0).finished();
}

Eigen::Matrix<double,4,4> projectionClipFromImplRdf(
    double L, double R, double T, double B,
    double zNear, double zFar
) {
    Eigen::Matrix<double,4,4> m = Eigen::Matrix<double,4,4>::Zero();
    m(0,0) = 2 * zNear / (R-L);
    m(1,1) = 2 * zNear / (T-B);
    m(0,2) = (R+L)/(L-R);
    m(1,2) = (T+B)/(B-T);
    m(2,2) = (zFar +zNear) / (zFar - zNear);
    m(3,2) = 1.0;
    m(2,3) =  (2*zFar*zNear)/(zNear - zFar);
    return m;
}

Eigen::Matrix<double,4,4> projectionClipFromImplRub(
    double L, double R, double T, double B,
    double zNear, double zFar
) {
    Eigen::Matrix<double,4,4> m = Eigen::Matrix<double,4,4>::Zero();
    m(0,0) = 2 * zNear / (R-L);
    m(1,1) = 2 * zNear / (T-B);
    m(0,2) = (R+L)/(R-L);
    m(1,2) = (T+B)/(T-B);
    m(2,2) = -(zFar +zNear) / (zFar - zNear);
    m(3,2) = -1.0;
    m(2,3) =  -(2*zFar*zNear)/(zFar-zNear);
    return m;
}

Eigen::Matrix<double,4,4> projectionClipFromCamera(
    sophus::ImageSize size,
    Eigen::Vector2d focal_distance_pixels,
    Eigen::Vector2d principle_point,
    MinMax<double> near_far_in_world_units,
    DeviceXyz coord_convention,
    ImageXy image_convention,
    ImageIndexing image_indexing
) {
    if(image_indexing != ImageIndexing::pixel_centered) {
        PANGO_UNIMPLEMENTED();
        PANGO_FATAL();
    }

    const double u0 = principle_point.x();
    const double v0 = principle_point.y();
    const double zNear = near_far_in_world_units.min();
    const double zFar = near_far_in_world_units.max();
    const double fu = focal_distance_pixels.x();
    const double fv = focal_distance_pixels.y();
    const double w = size.width;
    const double h = size.height;

    if(coord_convention == DeviceXyz::right_down_forward && image_convention == ImageXy::right_down) {
        const double L = -(u0) * zNear / fu;
        const double R = +(w-u0) * zNear / fu;
        const double T = -(v0) * zNear / fv;
        const double B = +(h-v0) * zNear / fv;
        return projectionClipFromImplRdf(L, R, T, B, zNear, zFar);
    }else
    if(coord_convention == DeviceXyz::right_down_forward && image_convention == ImageXy::right_up) {
        const double L = -(u0) * zNear / fu;
        const double R = +(w-u0) * zNear / fu;
        const double B = -(v0) * zNear / fv;
        const double T = +(h-v0) * zNear / fv;
        return projectionClipFromImplRdf(L, R, T, B, zNear, zFar);
    }else
    if(coord_convention == DeviceXyz::right_up_back && image_convention == ImageXy::right_up) {
        const double L = +(u0) * zNear / -fu;
        const double T = +(v0) * zNear / fv;
        const double R = -(w-u0) * zNear / -fu;
        const double B = -(h-v0) * zNear / fv;
        return projectionClipFromImplRub(L, R, T, B, zNear, zFar);
    }else
    if(coord_convention == DeviceXyz::right_up_back && image_convention == ImageXy::right_down) {
        const double L = +(u0) * zNear / -fu;
        const double R = -(w-u0) * zNear / -fu;
        const double T = -(h-v0) * zNear / fv;
        const double B = +(v0) * zNear / fv;
        return projectionClipFromImplRub(L, R, T, B, zNear, zFar);
    }else{
        PANGO_UNIMPLEMENTED();
        PANGO_FATAL();
    }
}

Eigen::Matrix<double,4,4> projectionClipFromOrtho(
    MinMax<Eigen::Vector2d> extent,
    MinMax<double> near_far_in_world_units,
    ImageXy image_convention,
    ImageIndexing image_indexing
) {
    Eigen::Matrix4d m = Eigen::Matrix4d::Zero();

    double l = extent.min().x();
    double r = extent.max().x();
    double b = extent.max().y(); // assuming 0 is top
    double t = extent.min().y(); // assuming 0 is top
    const double n = near_far_in_world_units.min();
    const double f = near_far_in_world_units.max();

    // fix assumption if needed
    if(image_convention == ImageXy::right_up)
        std::swap(b,t);

    //
    if(image_indexing == ImageIndexing::pixel_centered) {
        l -= 0.5; r -= 0.5;
        b -= 0.5; t -= 0.5;
    }else if(image_indexing != ImageIndexing::pixel_continuous) {
        // nothing to do.
    }

    m(0, 0) = 2.0 / (r-l);
    // m(1, 0) = 0;
    // m(2, 0) = 0;
    // m(3, 0) = 0;

    // m(0, 1) = 0;
    m(1, 1) = 2.0 / (t-b);
    // m(2, 1) = 0;
    // m(3, 1) = 0;

    // m(0, 2) = 0;
    // m(1, 2) = 0;
    m(2, 2) = -2.0 / (f-n);
    // m(3, 2) = 0;

    m(0, 3) = -(r+l) / (r-l);
    m(1, 3) = -(t+b) / (t-b);
    m(2, 3) = -(f+n) / (f-n);
    m(3, 3) = 1.0;

    return m;
}

}
