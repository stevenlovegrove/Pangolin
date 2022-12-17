#include <pangolin/context/context.h>
#include <pangolin/gui/all_layers.h>
#include <pangolin/maths/camera_look_at.h>
#include <pangolin/gui/make_drawable.h>

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
        .camera_from_world = cameraLookatFromWorld(
            {0.0,0.0,1.0}, {10.0,0.0,0.0}, AxisDirection2::positive_z
        ),
        .near_far = {0.01, 1000.0}
    });
    auto checker_plane = DrawnSolids::Create({});

    static_assert(sizeof(sophus::SE3f) == 32);
    std::vector<sophus::SE3f> parent_from_axis_poses;
    parent_from_axis_poses.emplace_back();
    // up 1m
    translate(parent_from_axis_poses, {0.0f, 0.0, 1.0}, 10);
    // along x-axis 1m
    translate(parent_from_axis_poses, {1.0f, 0.0, 0.0}, 10);
    // rotate 90 degrees clockwise around z axis
    rotate(parent_from_axis_poses, {0.0, 0.0, -M_PI/2.0}, 10);
    // along x-axis 1m
    translate(parent_from_axis_poses, {1.0f, 0.0, 0.0}, 10);

    // Spiral
    for(int i=0; i < 100; ++i) {
        const sophus::SE3f delta(sophus::SO3f::rotX(0.1)*sophus::SO3f::rotY(0.05), Eigen::Vector3f(0.1, 0.2f, 0.0f));
        parent_from_axis_poses.push_back( parent_from_axis_poses.back() * delta );
    }

    sophus::SE3d scene_from_parent = sophus::Se3F64::transZ(0.2);
    scene->addInScene(checker_plane);
    scene->addInSceneAt(draw::Axes{.drawable_from_axis_poses = parent_from_axis_poses}, scene_from_parent);

    context->setLayout( scene);

    context->loop([&](){
        return true;
    });
    return 0;
}
