#include <pangolin/context/context.h>
#include <pangolin/gui/all_layers.h>
#include <pangolin/gui/make_drawable.h>
#include <pangolin/maths/camera_look_at.h>

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

  auto widgets = WidgetLayer::Create(
      {.name = "ui.", .size_hint = {Pixels{250}, Parts{1}}});

  auto scene = DrawLayer::Create(
      {.camera_from_world = cameraLookatFromWorld(
           {0.0, 0.0, 1.0}, {10.0, 0.0, 0.0}, AxisDirection2::positive_z),
       .near_far = {0.01, 1000.0}});
  context->setLayout(widgets | scene);

  scene->addInScene(draw::CheckerPlane{});
  scene->addNamedInScene("axis", draw::Axis{.scale = 5.f});

  VarSe3 world_from_box(
      "ui.world_from_box", Eigen::Vector3d::Zero(), Eigen::Vector3d::Zero(), 1,
      1);

  context->loop([&]() {
    scene->addNamedInSceneAt(
        "world_from_box", draw::Cube{}, world_from_box.get());

    return true;
  });
  return 0;
}
