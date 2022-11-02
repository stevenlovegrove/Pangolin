#include <pangolin/context/context.h>
#include <pangolin/gui/render_layer_group.h>
#include <pangolin/gui/draw_layer.h>
#include <pangolin/gui/widget_layer.h>
#include <pangolin/handler/handler.h>
#include <pangolin/maths/camera_look_at.h>
#include <sophus/image/image.h>
#include <sophus/sensor/camera_model.h>
#include <pangolin/var/var.h>
#include <pangolin/image/image_io.h>

using namespace pangolin;
using namespace sophus;

void newApi()
{
    auto context = Context::Create({
        .title="Minimal Example",
        .window_size = {1024,600},
    } );

    auto video_feed = RenderableImage::Create({
        .image=LoadImage("/Users/stevenlovegrove/Desktop/headshots/1.png")
    });
    video_feed->image->sync();

    auto image_layer = DrawLayer::Create({.title="Important Thing2", .objects={video_feed}});

    auto cam_view =
        image_layer ^
        DrawLayer::Create({.objects={video_feed}});

    auto plot = DrawLayer::Create({.title="plot", .objects={video_feed}});

    auto panel = WidgetLayer::Create({
        .size_hint = {Pixels{300}, Parts{1}}
    });

    context->setLayout(panel |  (cam_view / plot ));



    Var<float> test1("ui.slider1", 20.0, 0.0, 50.0);
    Var<int> test2("ui.slider2", 3, 0, 15);
    Var<double> test3("ui.slider3", 3.2e-1, 0.0, 1.0);
    Var<bool> test4("ui.button", false, false);
    Var<bool> test5("ui.checkbox", false, true);
    Var<std::string> test7("ui.textbox", "Hello");

    context->loop();
}

int main( int /*argc*/, char** /*argv*/ )
{
    newApi();
    return 0;
}
