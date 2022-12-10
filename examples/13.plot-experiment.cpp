#include <pangolin/context/context.h>
#include <pangolin/gui/all_layers.h>
#include <sophus/sensor/orthographic.h>

using namespace pangolin;

/// Returns orthographic camera model given bounding box and image size.
template <class TScalar>
sophus::OrthographicModelT<TScalar> plot_camera(
    Eigen::AlignedBox<TScalar, 2> const& bounding_box) {
    // Plotter is not discretized into pixels, so we use a unit width and
    // height

  Eigen::Array<TScalar, 2, 1> const range =
      bounding_box.max() - bounding_box.min();
  Eigen::Array<TScalar, 2, 1> const scale =
      Eigen::Array<TScalar, 2, 1>(1.0, 1.0) / range;
  Eigen::Array<TScalar, 2, 1> const offset =
      -(scale*bounding_box.min().array());

  Eigen::Matrix<TScalar, 4, 1> const params(
      scale.x(), scale.y(), offset.x(), offset.y());

  return sophus::OrthographicModelT<TScalar>({1,1}, params);
}

int main( int /*argc*/, char** /*argv*/ )
{
    // Make some data
    std::vector<Eigen::Vector2f> plot_data;

    for(double x=0; x < 100.0; x += 0.2) {
        float y = std::sin(x);
        plot_data.emplace_back(x, y);
    }

    // Create application window
    auto context = Context::Create({ .title="Plot experiment", } );

    // Create a drawable with line series
    auto graph_xy = DrawnPrimitives::Create({
        .element_type = DrawnPrimitives::Type::line_strip
    });
    graph_xy->vertices->update(plot_data, {});

    // Create point markers
    std::vector<uint16_t> plot_shape;
    std::vector<Eigen::Vector4f> plot_color;
    for(auto& x : plot_data) {
        plot_shape.push_back(10);
        plot_color.emplace_back(0.2, 0.6, 0.5, 1.0);
    }

    auto markers = DrawnPrimitives::Create({
        .element_type = DrawnPrimitives::Type::shapes,
        .default_size = 0.1,
    });
    markers->vertices->update(plot_data, {});
    markers->shapes->update(plot_shape, {});
    markers->colors->update(plot_color, {});

    auto layer = DrawLayer::Create({
        .aspect_policy = AspectPolicy::stretch,
        .image_convention = ImageXy::right_up,
        .image_indexing = ImageIndexing::normalized_zero_one,
        .handler = DrawLayerHandler::Create({.view_mode = ViewMode::image_plane}),
        .camera = plot_camera(Eigen::AlignedBox2d{
            Eigen::Vector2d(0.0, -1.0),
            Eigen::Vector2d(10.0, 1.0)
        }),
        .in_scene = {graph_xy, markers}
    });

    context->setLayout(layer);
    context->loop();
    return 0;
}
