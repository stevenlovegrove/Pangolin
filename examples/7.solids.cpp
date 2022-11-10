#include <pangolin/context/context.h>
#include <pangolin/gui/widget_layer.h>
#include <pangolin/video/video.h>
#include <pangolin/var/var.h>
#include <pangolin/gl/glsl_program.h>
#include <pangolin/render/gl_vao.h>
#include <pangolin/maths/camera_look_at.h>
/*
  == Pangolin-by-example ==

*/

using namespace pangolin;
using namespace sophus;

int main( int argc, char** argv )
{
    auto context = Context::Create({
        .title="Pangolin Solid Raycast Geometry",
        .window_size = {2*640,2*480},
    } );

    auto scene = DrawLayer::Create({
        .camera_from_world = std::make_shared<sophus::Se3F64>(
            cameraLookatFromWorld({0.0,0.0,1.0}, {10.0,0.0,0.0}, AxisDirection2::positive_z)
        )
    });
    auto solids = DrawnSolids::Create({});
    scene->add(solids);

    context->setLayout( scene);

    context->loop([&](){
        return true;
    });
    return 0;
}
