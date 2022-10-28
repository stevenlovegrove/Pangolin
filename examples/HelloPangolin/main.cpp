#include <pangolin/context/context.h>
#include <pangolin/gui/panel_group.h>
#include <pangolin/gui/render_panel.h>
#include <sophus/image/image.h>

using namespace pangolin;
using namespace sophus;

Image<float> makeAnImage()
{
    MutImage<float> img;
    img.fill(0.5f);
    return img;
}

// TODO: really important that we show how to get to raw opengl
//       rendering easily...

void newApi()
{
    using namespace pangolin;

    Shared<Context> context = Context::Create({
        .title="Minimal Example",
        .window_size = {640, 480},
    } );

    auto multi = RenderPanel::Create({});
    context->setLayout(multi);


    context->loop([](){
        return true;
    });
}

int main( int /*argc*/, char** /*argv*/ )
{
    newApi();
    return 0;
}
