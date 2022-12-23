#include <pangolin/context/context.h>
#include <pangolin/gl/color.h>
#include <pangolin/gui/all_layers.h>
#include <pangolin/image/image_io.h>

using namespace pangolin;

// Demonstrate use of the 'shapes' primitives for rendering anti-alised markers.
// TODO: We'll add some convenience methods for using these more easily.
int main(int /*argc*/, char** /*argv*/)
{
  auto context = Context::Create({
      .title = "Pangolin Shapes",
      .window_size = {1024, 600},
  });

  auto primitives = DrawnPrimitives::Create(
      {.element_type = DrawnPrimitives::Type::shapes, .default_size = 10.0});

  std::vector<Eigen::Vector3f> points;
  std::vector<Eigen::Vector4f> colors;
  std::vector<uint16_t> shapes;
  const int N = 11 * 2;
  ColorWheel wheel;

  for (int i = 0; i < N; ++i) {
    auto c = wheel.GetColorBin(i);
    points.push_back({float(i), 0.0, 0.0});
    colors.push_back({c.r, c.g, c.b, 1.0});
    shapes.push_back(i);
  }

  primitives->vertices->update(points);
  primitives->colors->update(colors);
  primitives->shapes->update(shapes);

  context->setLayout(primitives);
  context->loop();
  return 0;
}
