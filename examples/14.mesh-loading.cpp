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
  CLI::App app{"Pangolin Mesh Loading Example"};
  app.add_option("-f,--file", mesh_file, "Path to 3D model")->required();
  app.add_option("-m,--matcap", matcap_file, "Path to matcap image");
  CLI11_PARSE(app, argc, argv);

  auto context = Context::Create({
      .title = "Pangolin Mesh Loading",
      .window_size = {2 * 640, 2 * 480},
  });

  // objects to draw
  auto checker_plane = makeDrawable(draw::CheckerPlane{});
  auto mesh = DrawnGroup::Create({.file_assets = mesh_file});
  if (!matcap_file.empty()) {
    auto material_image = DeviceTexture::Create({});
    material_image->update(LoadImage(matcap_file));
    forAllT<DrawnPrimitives>(mesh->children, [&](DrawnPrimitives& p) {
      p.material_image = material_image;
    });
  }

  auto scene = DrawLayer::Create(
      {.camera_from_world = cameraLookatFromWorld(
           {0.0, 0.0, 1.0}, {10.0, 0.0, 0.0}, AxisDirection2::positive_z),
       .in_scene = {checker_plane, mesh}});

  context->setLayout(scene);

  context->loop();
  return 0;
}
