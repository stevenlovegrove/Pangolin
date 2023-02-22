#include <pangolin/context/context.h>
#include <pangolin/layer/all_layers.h>
#include <pangolin/drawable/drawn_group.h>
#include <pangolin/drawable/make_drawable.h>
#include <pangolin/render/camera_look_at.h>

/*
  == Pangolin-by-example ==

*/

using namespace pangolin;
using namespace sophus;

int main(int argc, char** argv)
{
  auto context = Context::Create({
      .title = "Pangolin Solid Raycast Geometry",
      .window_size = {2 * 640, 2 * 480},
  });

  auto widget_layer =
      WidgetLayer::Create({.size_hint = {Pixels{200}, Parts{1}}});

  // objects to draw
  auto checker_plane = makeDrawable(draw::CheckerPlane{});
  auto axis = makeDrawable(sophus::SE3f());
  auto star =
      makeDrawable(draw::Shape{.type = DrawnPrimitives::Shape::hollow_star});
  auto heart =
      makeDrawable(draw::Shape{.type = DrawnPrimitives::Shape::hollow_heart});
  auto square =
      makeDrawable(draw::Shape{.type = DrawnPrimitives::Shape::hollow_box});

  // Set the position of the star within its group
  star->pose.parent_from_drawable = sophus::SE3d::trans(1.0, 0.0, 1.0);

  auto scene = DrawLayer::Create(
      {.camera_from_world = cameraLookatFromWorld(
           {0.0, 0.0, 1.0}, {10.0, 0.0, 0.0}, AxisDirection2::positive_z),
       .in_scene = {checker_plane, axis}});

  // group some objects
  auto group1 = scene->addInSceneAt(
      DrawnGroup::Create({
          .children = {axis, star, square},
      }),
      sophus::SE3d::trans(5.0, 0.0, 1.0));

  auto group2 = scene->addInSceneAt(
      DrawnGroup::Create({
          .children = {axis, heart},
      }),
      sophus::SE3d::trans(4.0, 1.0, 1.0));

  context->setLayout(widget_layer | scene);

  // Alow the position of group one (containing the axis, star and square) to
  // be manipulated in the gui.
  Var<double>::Attach(
      "group1-x", group1->pose.parent_from_drawable.translation().x(), -10.0,
      10.0);
  Var<double>::Attach(
      "group1-y", group1->pose.parent_from_drawable.translation().y(), -10.0,
      10.0);

  context->loop();
  return 0;
}
