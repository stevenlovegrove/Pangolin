#include <pangolin/context/context.h>
#include <pangolin/layer/all_layers.h>
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

  auto widgets = WidgetLayer::Create(
      {.name = "ui.", .size_hint = {Pixels{220}, Parts{1}}});

  auto scene = DrawLayer::Create(
      {.camera_from_world = cameraLookatFromWorld(
           {0.0, 0.0, 1.0}, {10.0, 0.0, 0.0}, AxisDirection2::positive_z),
       .near_far = {0.01, 1000.0}});
  std::string unique_name = "foo";

  Var<std::function<void(void)>> ui_delete(
      "ui.delete", [&]() { scene->remove(unique_name); });

  Var<std::function<void(void)>> ui_add_plane("ui.add_plane", [&]() {
    scene->addNamedInScene(unique_name, draw::CheckerPlane{});
  });

  Var<std::function<void(void)>> ui_add_axis("ui.add_axis", [&]() {
    scene->addNamedInScene(unique_name, sophus::SE3f::transX(1));
  });

  Var<std::function<void(void)>> ui_add_cube("ui.add_cube", [&]() {
    scene->addNamedInSceneAt(
        unique_name, pangolin::draw::Cube{},
        sophus::SE3d::trans(5.0, 0.0, 1.0));
  });

  Var<std::function<void(void)>> ui_add_camera("ui.add_camera", [&]() {
    scene->addNamedInSceneAt(
        unique_name,
        pangolin::draw::CameraFrustum{
            .camera = sophus::CameraModel::createDefaultPinholeModel(
                sophus::ImageSize(640, 480))},
        sophus::SE3d::trans(5.0, 0.0, 1.0));
  });

  scene->addNamedInScene(unique_name, draw::CheckerPlane{});

  context->setLayout(widgets | scene);

  context->loop([&]() { return true; });
  return 0;
}
