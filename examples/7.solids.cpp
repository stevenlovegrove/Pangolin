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
    auto prims = DrawnPrimitives::Create({
        .element_type = DrawnPrimitives::Type::axes
    });

    static_assert(sizeof(sophus::SE3f) == 32);
    auto T_vert_draw = std::make_shared<std::vector<sophus::SE3f>>();
    T_vert_draw->push_back(sophus::SE3f());

    for(int i=0; i < 100; ++i) {
        const sophus::SE3f delta(sophus::SO3f::rotX(0.1)*sophus::SO3f::rotY(0.05), Eigen::Vector3f(0.1, 0.2f, 0.0f));
        T_vert_draw->push_back( T_vert_draw->back() * delta );
    }
    prims->vertices->update({
        .data = std::shared_ptr<void>(T_vert_draw, T_vert_draw->data()),
        .data_type = {NumberType::floating_point, 8, 4},
        .num_elements = T_vert_draw->size()
    });

    scene->add(solids, prims);

    context->setLayout( scene);

    context->loop([&](){
        return true;
    });
    return 0;
}
