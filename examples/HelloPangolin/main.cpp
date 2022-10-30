#include <pangolin/context/context.h>
#include <pangolin/gui/render_layer_group.h>
#include <pangolin/gui/draw_layer.h>
#include <pangolin/handler/handler.h>
#include <pangolin/maths/camera_look_at.h>
#include <sophus/image/image.h>
#include <sophus/sensor/camera_model.h>

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

    context->setLayout(DrawLayer::Create({
        .handler = Handler::Create({
            .camera = copyShared(camera),
            .world_from_camera = copyShared(worldLookatFromCamera(
                {0,0,10}, {0,10,0}
            ))
        }),
        // .objects = Object::Create(Pangolin)
    }));

    context->loop();
}

int main( int /*argc*/, char** /*argv*/ )
{
    newApi();
    return 0;
}
