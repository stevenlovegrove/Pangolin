
#include <pangolin/gui/make_drawable.h>

namespace pangolin{

Shared<Drawable> DrawableConversionTraits<Shape>::makeDrawable(const Shape& x) {
    auto prims = DrawnPrimitives::Create({
        .element_type=DrawnPrimitives::Type::shapes,
        .default_size = x.size,
    });
    prims->vertices->update(std::vector<Eigen::Vector3f>{ x.pos.cast<float>() }, {});
    prims->shapes->update(std::vector<uint16_t>{ static_cast<uint16_t>(x.type) }, {});
    prims->colors->update(std::vector<Eigen::Vector4f>{ x.color.cast<float>() }, {});
    return prims;
}

Shared<Drawable> DrawableConversionTraits<sophus::Se3F32>::makeDrawable(const sophus::Se3F32& x) {
    auto prims = DrawnPrimitives::Create({
        .element_type=DrawnPrimitives::Type::axes,
    });
    prims->vertices->update(std::vector<sophus::Se3<float>>{x}, {});
    return prims;
}
}