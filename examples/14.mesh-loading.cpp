#include <CLI/CLI11.hpp>
#include <pangolin/context/context.h>
#include <pangolin/drawable/drawn_group.h>
#include <pangolin/drawable/make_drawable.h>
#include <pangolin/image/image_io.h>
#include <pangolin/layer/all_layers.h>
#include <pangolin/render/camera_look_at.h>
#include <pangolin/utils/for_all_t.h>

/*
  == Pangolin-by-example ==

*/

using namespace pangolin;
using namespace sophus;

int main(int argc, char** argv)
{
  std::filesystem::path mesh_file;
  std::filesystem::path matcap_file;
  double scale = 1.0;
  bool with_grid = false;
  CLI::App app{"Pangolin Mesh Loading Example"};
  app.add_option("-f,--file", mesh_file, "Path to 3D model")->required();
  app.add_option("-m,--matcap", matcap_file, "Path to matcap image");
  app.add_flag("-g,--grid", with_grid, "Show grid floor in scene (z=0)");
  app.add_option(
      "-s,--scale", scale,
      "Scale that should be applied to model vertices to bring into scene "
      "units.");
  CLI11_PARSE(app, argc, argv);

  auto context = Context::Create({
      .title = "Pangolin Mesh Loading",
      .window_size = {2 * 640, 2 * 480},
  });

  // Load and process mesh from file.
  auto mesh = DrawnGroup::Load(mesh_file, {});
  if (!matcap_file.empty()) {
    auto material_image = DeviceTexture::Create({});
    material_image->update(LoadImage(matcap_file));
    forAllT<DrawnPrimitives>(mesh->children, [&](DrawnPrimitives& p) {
      p.material_image = material_image;
      p.pose.parent_from_drawable.setScale(scale);
    });
  }

  // Create scene to render
  auto scene = DrawLayer::Create(
      {.camera_from_world = cameraLookatFromWorld(
           {2.0, 0.0, 2.0}, {0.0, 0.0, 0.0}, AxisDirection::positive_z),
       });
  if(with_grid) {
    scene->addInScene(draw::CheckerPlane{});
  }
  scene->addInScene(mesh);

  context->setLayout(scene);

  context->loop();
  return 0;
}
