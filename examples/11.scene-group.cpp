#include <pangolin/context/context.h>
#include <pangolin/gui/all_layers.h>
#include <pangolin/maths/camera_look_at.h>
#include <pangolin/gui/drawn_group.h>
/*
  == Pangolin-by-example ==

*/

using namespace pangolin;
using namespace sophus;

namespace pangolin{

struct Shape
{
    Eigen::Vector3d pos = {0.0, 0.0, 0.0};
    Eigen::Vector4d color = {1.0, 0.3, 0.5, 1.0};
    double size = 1.0;
    DrawnPrimitives::Shape type = DrawnPrimitives::Shape::hollow_star;
};

// Implement traits so that Pangolin knows how to render the type.
template<>
struct DrawableConversionTraits<Shape> {
static Shared<Drawable> makeDrawable(const Shape& x) {
    auto prims = DrawnPrimitives::Create({
        .element_type=DrawnPrimitives::Type::shapes,
        .default_size = x.size,
    });
    prims->vertices->update(std::vector<Eigen::Vector3f>{ x.pos.cast<float>() }, {});
    prims->shapes->update(std::vector<uint16_t>{ static_cast<uint16_t>(x.type) }, {});
    prims->colors->update(std::vector<Eigen::Vector4f>{ x.color.cast<float>() }, {});
    return prims;
}};

// Implement traits so that Pangolin knows how to render the type.
template<typename T>
struct DrawableConversionTraits<sophus::Se3<T>> {
static Shared<Drawable> makeDrawable(const sophus::Se3<T>& x) {
    auto prims = DrawnPrimitives::Create({
        .element_type=DrawnPrimitives::Type::axes,
    });
    prims->vertices->update(std::vector<sophus::Se3<T>>{x}, {});
    return prims;
}};
}

int main( int argc, char** argv )
{
    auto context = Context::Create({
        .title="Pangolin Solid Raycast Geometry",
        .window_size = {2*640,2*480},
    } );

    auto widget_layer = WidgetLayer::Create({
        .size_hint = {Pixels{200}, Parts{1}}
    });

    // objects to draw
    auto checker_plane = DrawnSolids::Create({});
    auto axis = makeDrawable(sophus::SE3f());
    auto star = makeDrawable(Shape{ .type=DrawnPrimitives::Shape::hollow_star });
    auto heart = makeDrawable(Shape{.type=DrawnPrimitives::Shape::hollow_heart});
    auto square = makeDrawable(Shape{.type=DrawnPrimitives::Shape::hollow_box});

    // group some objects
    auto group1 = DrawnGroup::Create({
        .children = {axis, star, square},
        .parent_from_drawable = sophus::SE3d::trans(5.0, 0.0, 1.0).matrix()
    });
    auto group2 = DrawnGroup::Create({
        .children = {axis, heart},
        .parent_from_drawable = sophus::SE3d::trans(4.0, 1.0, 1.0).matrix()
    });

    // Set the position of the star within its group
    star->parent_from_drawable = sophus::SE3d::trans(1.0, 0.0, 1.0).matrix();

    auto scene = DrawLayer::Create({
        .camera_from_world = cameraLookatFromWorld(
            {0.0,0.0,1.0}, {10.0,0.0,0.0}, AxisDirection2::positive_z
        ),
        .in_scene = {checker_plane, axis, group1, group2}
    });

    context->setLayout( widget_layer | scene);

    // Alow the position of group one (containing the axis, star and square) to
    // be manipulated in the gui.
    Var<double>::Attach("group1-x", group1->parent_from_drawable(0,3), -10.0, 10.0);
    Var<double>::Attach("group1-y", group1->parent_from_drawable(1,3), -10.0, 10.0);

    context->loop();
    return 0;
}
