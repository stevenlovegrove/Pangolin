#include <pangolin/context/context.h>
#include <pangolin/gui/widget_layer.h>
#include <pangolin/var/var.h>
#include <sophus/lie/so3.h>

/*
  == Pangolin-by-example ==

*/

using namespace pangolin;
using namespace sophus;

int main( int /*argc*/, char** /*argv*/ )
{
    auto context = Context::Create({
        .title="Pangolin Widgets",
    } );

    auto widgets = WidgetLayer::Create({
        .name="ui.",
        .size_hint={Pixels{180}, Parts{1}}
    });

    auto primitives = DrawnPrimitives::Create();

    primitives->vertices->update(
        std::vector<Eigen::Vector3f>{
            {-2.0f, -2.0f, 0.0f},
            { 2.0f, -2.0f, 0.0f },
            { 0.0f,  2.0f, 0.0f }
        });

    // TODO: Pangolin Vars will probably get modernized soon...
    Var<double> angle_theta("ui.theta", 0.0, -sophus::kPi<double>, +sophus::kPi<double>);
    Var<bool> filled("ui.filled", true, true);
    Var<double> color_red("ui.red", 0.6, 0.0, 1.0);
    Var<double> color_green("ui.green", 0.3, 0.0, 1.0);
    Var<double> color_blue("ui.blue", 0.4, 0.0, 1.0);

    context->setLayout( widgets | primitives );

    context->loop([&](){
        primitives->default_color
            = Eigen::Vector4d(color_red, color_green, color_blue, 1.0);

        primitives->point_type = filled ?
             DrawnPrimitives::Type::triangles :
             DrawnPrimitives::Type::line_loop;

        primitives->world_from_drawable = sophus::SE3d(
            sophus::SO3d::rotZ(angle_theta),
            Eigen::Vector3d::Zero()).matrix();
        return true;
    });

    return 0;
}
