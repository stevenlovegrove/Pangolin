#include <pangolin/gui/drawn_primitives.h>
#include <pangolin/gui/drawn_image.h>
#include <pangolin/context/context.h>

/*
  == Pangolin-by-example ==

*/

using namespace pangolin;
using namespace sophus;

sophus::Image<float> testImage(int w, int h)
{
    sophus::MutImage<float> img(ImageSize(8,5));
    for(int y=0; y < h; ++y) {
        for(int x=0; x < w; ++x) {
            img.checkedMut(x,y) = float(y*w + x) / float(w*h);
        }
    }
    return std::move(img);
}

int main( int argc, char** argv )
{
    const int w = 8;
    const int h = 5;
    const int win_scale = 100;
    auto context = Context::Create({
        .title="Pixel-perfect overlay",
        .window_size = {win_scale*w, win_scale*h},
    } );

    // Draw a very low resolution image so that we can visually ensure we are
    // not missing any half pixels etc
    auto image = testImage(w, h);

    // draw a red cross centered at (0,0), which should be the center of the
    // top-left pixel of the image. The color of the pixel should be black
    auto cross_top_left = DrawnPrimitives::Create({
        .element_type=DrawnPrimitives::Type::lines,
        .enable_visibility_testing = false,
        .default_color = {1.0, 0.0, 0.0, 1.0}
    });
    cross_top_left->vertices->update(std::vector<Eigen::Vector3f>{
        {-0.5f, -0.5f, 0.0f}, {+0.5f, +0.5f, 0.0f},
        {-0.5f, +0.5f, 0.0f}, {+0.5f, -0.5f, 0.0f}
    }, {});

    // draw a black cross centered at (w-1,h-1), which should be the center of
    // the bottom-right pixel of the image. The color of the pixel should be
    // white
    auto cross_bottom_right = DrawnPrimitives::Create({
        .element_type=DrawnPrimitives::Type::lines,
        .enable_visibility_testing = false,
        .default_color = {0.0, 0.0, 0.0, 1.0}
    });
    cross_bottom_right->vertices->update(std::vector<Eigen::Vector3f>{
        {w-1.0f-0.5f, h-1.0f-0.5f, 0.0f}, {w-1.0f+0.5f, h-1.0f+0.5f, 0.0f},
        {w-1.0f-0.5f, h-1.0f+0.5f, 0.0f}, {w-1.0f+0.5f, h-1.0f-0.5f, 0.0f}
    }, {});

    // Place the elements in one DrawnLayer object which will share the same
    // camera matrices. The object will 'guess' an orthographic projection
    // across the image is what we intended since it is listed first in the
    // .objects list.
    auto layer = DrawLayer::Create({
        .objects_in_camera = { DrawnImage::Create({.image=image}), cross_top_left, cross_bottom_right },
    });

    context->setLayout( layer );

    context->loop();
    return 0;
}
