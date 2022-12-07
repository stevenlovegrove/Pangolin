#include <pangolin/context/context.h>
#include <pangolin/gui/all_layers.h>
#include <pangolin/maths/camera_look_at.h>
/*
  == Pangolin-by-example ==

*/

using namespace pangolin;
using namespace sophus;

inline void translate(std::vector<sophus::SE3f>& T_world_obj, const Eigen::Vector3f& dir_last, int steps)
{
    PANGO_ENSURE(T_world_obj.size() > 0);
    const SE3f T_last_new(SO3f(), dir_last / float(steps));

    for(int i=0; i < steps; ++i) {
        const auto& T_world_last = T_world_obj.back();
        T_world_obj.push_back( T_world_last * T_last_new);
    }
}

inline void rotate(std::vector<sophus::SE3f>& T_world_obj, const Eigen::Vector3f& axis_angle, int steps)
{
    PANGO_ENSURE(T_world_obj.size() > 0);
    const SE3f T_last_new(SO3f::exp(axis_angle/steps), Eigen::Vector3f::Zero());

    for(int i=0; i < steps; ++i) {
        const auto& T_world_last = T_world_obj.back();
        T_world_obj.push_back( T_world_last * T_last_new);
    }
}

int main( int argc, char** argv )
{
    auto context = Context::Create({
        .title="Pangolin Solid Raycast Geometry",
        .window_size = {2*640,2*480},
    } );

    auto scene = DrawLayer::Create({
        .camera = BrownConradyModel({1280,800}, {
            573.96337890625,
            573.96337890625,
            635.3743896484375,
            404.14239501953125,
            0.72640508413314819,
            -0.014841272495687008,
            1.3844746717950329e-05,
            0.00041974170017056167,
            -0.0051422379910945892,
            1.06774116e+00,
            1.28428817e-01,
            -1.99009608e-02
        }).binDown(),
        .camera_from_world = cameraLookatFromWorld(
            {0.0,0.0,1.0}, {10.0,0.0,0.0}, AxisDirection2::positive_z
        ),
        .near_far = {0.01, 1000.0}
    });
    auto checker_plane = DrawnSolids::Create({});
    auto prims = DrawnPrimitives::Create({
        .element_type = DrawnPrimitives::Type::axes,
        .default_size = 0.1
    });

    static_assert(sizeof(sophus::SE3f) == 32);
    std::vector<sophus::SE3f> T_world_axis;
    T_world_axis.emplace_back();
    // up 1m
    translate(T_world_axis, {0.0f, 0.0, 1.0}, 10);
    // along x-axis 1m
    translate(T_world_axis, {1.0f, 0.0, 0.0}, 10);
    // rotate 90 degrees clockwise around z axis
    rotate(T_world_axis, {0.0, 0.0, -M_PI/2.0}, 10);
    // along x-axis 1m
    translate(T_world_axis, {1.0f, 0.0, 0.0}, 10);

    // Spiral
    for(int i=0; i < 100; ++i) {
        const sophus::SE3f delta(sophus::SO3f::rotX(0.1)*sophus::SO3f::rotY(0.05), Eigen::Vector3f(0.1, 0.2f, 0.0f));
        T_world_axis.push_back( T_world_axis.back() * delta );
    }
    prims->vertices->update(T_world_axis, {});

    scene->addInScene(checker_plane, prims);

    context->setLayout( scene);

    context->loop([&](){
        return true;
    });
    return 0;
}
