#include <pangolin/context/context.h>
#include <pangolin/gui/all_layers.h>
#include <pangolin/image/image_io.h>
#include <pangolin/gl/colour.h>


using namespace pangolin;

struct Shapes {
  std::vector<DrawnPrimitives::Shape> shapes;
  std::vector<Eigen::Vector2f> pos2d;
  std::vector<Eigen::Vector3f> pos3d;
  std::vector<Eigen::Vector3f> colors;
  std::vector<Eigen::Vector4f> colorsalpha;

  DrawnPrimitives::Shape shape = DrawnPrimitives::Shape::hollow_circle;
  Eigen::Vector4f color = {1.0,1.0,1.0,1.0};
  float size = 10.0;

  struct Params {
    Eigen::Vector4f color;
    float size;
  };
  static Shapes circle(Eigen::Vector2f pos, float size, Eigen::Vector4f color) {
    return { .shapes = {DrawnPrimitives::Shape::filled_circle},
             .pos2d = {pos} };
  }
};

int main( int /*argc*/, char** /*argv*/ )
{
    auto context = Context::Create({
        .title="Pangolin Shapes",
        .window_size = {1024,600},
    } );

    // auto scene = DrawLayer::Create({
    //   .camera=sophus::createDefaultOrthoModel({640,480})
    // });

    // scene->addInPixels(
    //   Shapes({.pos={{0,0,0}, {640,480,0}}, .color})
    // );

    auto primitives = DrawnPrimitives::Create({
      .element_type=DrawnPrimitives::Type::shapes
    });

    std::vector<Eigen::Vector3f> points;
    std::vector<Eigen::Vector4f> colors;
    std::vector<uint16_t> shapes;
    const int N = 11*2;
    ColourWheel wheel;

    for(int i=0; i < N; ++i) {
      auto c = wheel.GetColourBin(i);
      points.push_back({double(i), 0.0, 0.0});
      colors.push_back({c.r, c.g, c.b, 1.0});
      shapes.push_back(i);
    }

    primitives->vertices->update(points, {});
    primitives->colors->update(colors, {});
    primitives->shapes->update(shapes, {});

    context->setLayout( primitives );
    context->loop();
    return 0;
}
