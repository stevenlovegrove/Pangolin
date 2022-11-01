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

    auto t1 = DrawLayer::Create({.title="Important Thing2", .objects={video_feed}});
    auto t2 = DrawLayer::Create({.title="Important Thing2"});
    auto t3 = DrawLayer::Create({.title="Important Thing3"});
    auto t4 = DrawLayer::Create({.title="Important Thing4"});
    auto panel = WidgetLayer::Create({
        .size_hint = {Pixels{300}, Parts{1}}
    });

    context->setLayout(panel | ((t1|t2) / ((t3|t1)|t4)) );



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
