#include <pangolin/context/context.h>
#include <pangolin/gl/color.h>
#include <pangolin/gui/all_layers.h>
#include <pangolin/gui/drawn_plot_background.h>
#include <sophus/sensor/orthographic.h>

using namespace pangolin;

/// Returns orthographic camera model given bounding box and image size.
template <class TScalar>
sophus::OrthographicModelT<TScalar> plot_camera(
    const Eigen::AlignedBox<TScalar, 2>& bounding_box)
{
  // Plotter is not discretized into pixels, so we use a unit width and
  // height

  const Eigen::Array<TScalar, 2, 1> range =
      bounding_box.max() - bounding_box.min();
  const Eigen::Array<TScalar, 2, 1> scale =
      Eigen::Array<TScalar, 2, 1>(1.0, 1.0) / range;
  const Eigen::Array<TScalar, 2, 1> offset =
      -(scale * bounding_box.min().array());

  const Eigen::Matrix<TScalar, 4, 1> params(
      scale.x(), scale.y(), offset.x(), offset.y());

  return sophus::OrthographicModelT<TScalar>({1, 1}, params);
}

int main(int /*argc*/, char** /*argv*/)
{
  // Make some data
  std::vector<Eigen::Vector2f> plot_data;

  for (double x = 0; x < 100.0; x += 0.2) {
    float y = std::sin(x);
    plot_data.emplace_back(x, y);
  }

  // Create application window
  auto context = Context::Create({
      .title = "Plot experiment",
  });

  // Create a drawable with line series
  auto graph_xy = DrawnPrimitives::Create(
      {.element_type = DrawnPrimitives::Type::line_strip, .default_size = 1.5});
  graph_xy->vertices->update(plot_data, {});

  if (0) {
    // Create point markers
    std::vector<uint16_t> plot_shape;
    std::vector<Color> plot_color;
    ColorWheel wheel;
    for (auto& x : plot_data) {
      plot_shape.push_back(wheel.GetCurrentIndex() % 21);
      plot_color.push_back(wheel.GetUniqueColor());
    }

    auto markers = DrawnPrimitives::Create({
        .element_type = DrawnPrimitives::Type::shapes,
        .default_size = 15,  // pixels
    });
    markers->vertices->update(plot_data, {});
    markers->shapes->update(plot_shape, {});
    markers->colors->update(plot_color, {});
  }

  auto bg = DrawnPlotBackground::Create(
      {.color_background = Eigen::Vector4f(0.97, 0.98, 1.0, 1.0)});

  auto layer = DrawLayer::Create(
      {.aspect_policy = AspectPolicy::stretch,
       .image_convention = ImageXy::right_up,
       .image_indexing = ImageIndexing::normalized_zero_one,
       .handler =
           DrawLayerHandler::Create({.view_mode = ViewMode::image_plane}),
       .camera = plot_camera(Eigen::AlignedBox2d{
           Eigen::Vector2d(-1.0, -1.1), Eigen::Vector2d(11.0, 1.1)}),
       .in_scene = {bg, graph_xy /* , markers */}});

  context->setLayout(layer);
  context->loop();
  return 0;
}
