#include <pangolin/context/context.h>
#include <pangolin/gui/render_layer_group.h>
#include <pangolin/gui/draw_layer.h>
#include <pangolin/gui/widget_layer.h>
#include <pangolin/handler/handler.h>
#include <pangolin/maths/camera_look_at.h>
#include <sophus/image/image.h>
#include <sophus/sensor/camera_model.h>
#include <pangolin/var/var.h>

using namespace pangolin;
using namespace sophus;

Image<float> makeAnImage()
{
    MutImage<float> img;
    img.fill(0.5f);
    return img;
}

void newApi()
{
    CameraModel camera = createDefaultPinholeModel({640,480});

    auto context = Context::Create({
        .title="Minimal Example",
        .window_size = camera.imageSize(),
    } );

    auto l1 = DrawLayer::Create({});
    auto l2 = DrawLayer::Create({});
    auto l3 = DrawLayer::Create({});
    auto l4 = DrawLayer::Create({});
    auto l5 = DrawLayer::Create({});

    auto l6 = WidgetLayer::Create({.size_hint = {
        RenderLayer::Absolute{300}, RenderLayer::Parts{1}
    }});

    auto layer = l6 | ((l1 | l2)/ (l3 | l4)) | l5;
    context->setLayout(layer);

    Var<float> test1("ui.slider1", 20.0, 0.0, 50.0);
    Var<int> test2("ui.slider2", 3, 0, 15);
    Var<double> test3("ui.slider3", 3.2e-1, 0.0, 1.0);
    Var<bool> test4("ui.button", false, false);
    Var<bool> test5("ui.checkbox", false, true);
    Var<std::function<void(void)>> test6("ui.button2", [](){
        static int i=0;
        std::cout << "Hello " << i++ << std::endl;
    });
    Var<std::string> test7("ui.textbox", "Hello");

    context->loop();
}

int main( int /*argc*/, char** /*argv*/ )
{
    newApi();
    return 0;
}
