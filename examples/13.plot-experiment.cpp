#include <pangolin/color/color.h>
#include <pangolin/context/context.h>
#include <pangolin/gui/all_layers.h>
#include <pangolin/gui/drawn_plot_background.h>

using namespace pangolin;

// Some global plot data for example
std::vector<Eigen::Vector2f> plot_data;

Shared<Drawable> sample_markers()
{
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
      .default_size = 15,
      .size_in_pixels = true,
  });
  markers->vertices->queueUpdate(plot_data);
  markers->shapes->queueUpdate(std::move(plot_shape));
  markers->colors->queueUpdate(std::move(plot_color));
  return markers;
}

Shared<Drawable> sample_text()
{
  auto test = DrawnText::Create({.font_height_pixels = 64});
  test->addText({2.0, 0.5}, "Hello");
  test->addText({3.0, 0.3}, "World!");
  return test;
}

int main(int /*argc*/, char** /*argv*/)
{
  // Make some data
  for (double x = 0; x < 1000.0; x += 0.2) {
    float y = std::sin(x/10.0);
    plot_data.emplace_back(x / 10.0, y);
  }

  // Create application window
  auto context = Context::Create({
      .title = "Plot experiment",
  });

  // Create a drawable with line series
  auto graph_xy = DrawnPrimitives::Create(
      {.element_type = DrawnPrimitives::Type::path, .default_size = 2, .size_in_pixels=true});
  graph_xy->vertices->queueUpdate(plot_data);

  std::optional<Shared<Drawable>> maybe_markers;
  std::optional<Shared<Drawable>> maybe_text;

  // maybe_markers = sample_markers();
  maybe_text = sample_text();

  auto bg = DrawnPlotBackground::Create(
      {.color_background = Eigen::Vector4f(0.97, 0.98, 1.0, 1.0)});

  auto layer = DrawLayer::Create(
      {.aspect_policy = AspectPolicy::stretch,
       .image_convention = ImageXy::right_up,
       .image_indexing = ImageIndexing::normalized_zero_one,
       .handler = DrawLayerHandler::Create(
           {.view_mode = ViewMode::image_plane,
            .constrain_image_zoom_bounds = false}),
       .camera = plot_camera(Eigen::AlignedBox2d{
           Eigen::Vector2d(-0.1, -1.1), Eigen::Vector2d(11.0, 1.1)}),
       .in_scene = {bg, graph_xy}});

  if (maybe_markers) {
    layer->addInScene(*maybe_markers);
  }
  if (maybe_text) {
    layer->addInScene(*maybe_text);
  }

  context->setLayout(layer);
  context->loop();
  return 0;
}
