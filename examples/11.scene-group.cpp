#include <pangolin/context/context.h>
#include <pangolin/gui/all_layers.h>
#include <pangolin/maths/camera_look_at.h>
#include <pangolin/gui/drawn_group.h>
#include <pangolin/gui/make_drawable.h>

/*
  == Pangolin-by-example ==

*/

using namespace pangolin;
using namespace sophus;



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
    auto star = makeDrawable(Draw::Shape{ .type=DrawnPrimitives::Shape::hollow_star });
    auto heart = makeDrawable(Draw::Shape{.type=DrawnPrimitives::Shape::hollow_heart});
    auto square = makeDrawable(Draw::Shape{.type=DrawnPrimitives::Shape::hollow_box});

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
