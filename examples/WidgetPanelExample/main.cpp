#include <pangolin/experimental/panel_group.h>
#include <pangolin/experimental/widget.h>
#include <pangolin/var/var.h>


using namespace pangolin;

void NewApi()
{
    using namespace pangolin;

    auto context = Context::Create({
        .title="Window Title",
        .window_size = {640, 480},
        .profile = Context::Profile::gl_3_2_core
    } );

    auto widget_panel = WidgetPanel::Create({});
    widget_panel += {
        Seperator::Create({"Section One"}),
        Slider::Create({"Slider", .min=0.0, .max=100.0}),
        // VarListener("ui.")
    };

    auto view_image = MultiPanel::Create({});
    auto view_3d = MultiPanel::Create({});
    auto view_image_2 = MultiPanel::Create({});
    auto view_3d_2 = MultiPanel::Create({});

    auto layer_2d_points = MultiPanel::Create({});
    auto camera_view_1 =  MultiPanel::Create({});

    // auto projection_3d_cam1;
    // auto projection_image_cam1;
    // auto composite = projection_3d_cam1 ^ layer_2d_points ^ camera_view_1;
        
    context->window() <  (widget_panel | ( ((view_image, view_3d_2) |   view_3d) /
                                           (view_image_2 | view_3d_2) ));

    // Var<double> v1("ui.v1");
    // Var<bool> v2("ui.v1");
    // Var<bool> v3("ui.v1");
    // Var<double> v4("ui.v1");

    // v1.changed() is some kind of an event.
    // v1.changed() || v2.changed() is also an event.
    // Changed(v1,v2,v3) is also an event

    // When(v1.changed() || v2.changed()).([](){

    // });

    context->loop([](){
        // anything we want to do custom in here
        return true;
    });
}

int main( int /*argc*/, char** /*argv*/ )
{
    NewApi();
    return 0;
}
