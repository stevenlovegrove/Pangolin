#include <pangolin/maths/projection.h>
#include <pangolin/utils/logging.h>

namespace pangolin
{

Eigen::Matrix<double,4,4> projectionClipFromCamera(
    sophus::ImageSize size,
    double focal_distance_pixels,
    Eigen::Vector2d principle_point,
    MinMax<double> near_far_in_world_units,
    DeviceXyz coord_convention,
    ImageXy image_convention,
    ImageIndexing image_indexing
) {
    PANGO_UNIMPLEMENTED();
    PANGO_FATAL();
}

Eigen::Matrix<double,4,4> projectionClipFromOrtho(
    MinMax<Eigen::Vector2d> extent,
    MinMax<double> near_far_in_world_units,
    ImageXy image_convention,
    ImageIndexing image_indexing
) {
    auto m = Eigen::Matrix<double,4,4>::Zero().eval();

    double l = extent.min().x();
    double r = extent.max().x();
    double b = extent.min().y(); // assuming 0 is bottom
    double t = extent.max().y(); // assuming 0 is bottom
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
        PANGO_UNIMPLEMENTED();
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
