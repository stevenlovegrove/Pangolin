#include <pangolin/context/context.h>
#include <pangolin/gui/layer_group.h>
#include <pangolin/gui/draw_layer.h>
#include <pangolin/gui/widget_layer.h>
#include <pangolin/handler/handler.h>
#include <pangolin/maths/camera_look_at.h>
#include <sophus/image/image.h>
#include <sophus/sensor/camera_model.h>
#include <pangolin/var/var.h>
#include <pangolin/image/image_io.h>
#include <pangolin/gl/glsl_program.h>

using namespace pangolin;
using namespace sophus;

void newApi()
{
    auto context = Context::Create({
        .title="Minimal Example",
        .window_size = {1024,600},
    } );

    auto im1 = LoadImage("https://www.wwf.org.uk/sites/default/files/styles/gallery_image/public/2019-09/pangolin_with_tongue_out.jpg?h=82f92a78&itok=tER1Azzc");
    auto im2 = LoadImage("https://avatars.githubusercontent.com/u/55272417?u=394e4c9779a714a3e7e5efa1a81618f8cc6893d9&v=4");
    auto imtall = LoadImage("https://upload.wikimedia.org/wikipedia/commons/thumb/2/23/Space_Needle_2011-07-04.jpg/500px-Space_Needle_2011-07-04.jpg");

    auto panel = WidgetLayer::Create({
        .size_hint = {Pixels{300}, Parts{1}}
    });

    auto primitives = DrawnPrimitives::Create();

    primitives->vertices->update(std::vector<Eigen::Vector3f>{
        {0.0f, 0.0f, 0.0f},
        {1.0f, 0.0f, 0.0f},
        {1.0f, 1.0f, 0.0f},
        {0.0f, 1.0f, 0.0f}
    });

    context->setLayout(panel |  (im1 / im2) | imtall | primitives);

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
