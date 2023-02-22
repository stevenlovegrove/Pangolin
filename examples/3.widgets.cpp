#include <pangolin/context/context.h>
#include <pangolin/layer/all_layers.h>
#include <pangolin/var/var.h>
#include <sophus/lie/so3.h>

/*
  == Pangolin-by-example ==

  Here we add some widgets to a 'WidgetLayer' that let us modify the simple
  rendered triangle graphic.

  For the WidgetLayer, we specify a .size_hint. Any layer can take these hints
  and they default to {Parts{1},Parts{1}}. They consist of a horizontal and
  vertical specification for the layers size as either a ratio in parts compared
  to other widgets, or in absolute pixel units. For example, for three widgets
  layed out horizontally, (X | Y | Z), if X a had a horizontal hint of Parts{1},
  Y of Parts{1} and Z of Parts{2}, then Z would have twice the width of either X
  or Y, and X and Y would have the same width.

  Here we also introduce pangolin::Vars. These are named and typed global
  variables which also contain some meta information about their ranges and use.
  Though they should be used sparingly, globals can have their place for
  exposing debug or prototype info through to a user quickly. pangolin::Vars do
  just that, and connect automatically to the WidgetLayer through the "ui."
  subscription prefix to allow us to tweak the render parameters.

  Finally, you can also see that we can specify a user-function to the loop
  context->loop() method which will be called after every frame. This callback
  occurs in the same main thread shared witht the context, so try not to do too
  much here or the GUI will slow down and become unresponsive. Here we use the
  callback to update the render parameters from the widgets.
*/

using namespace pangolin;
using namespace sophus;

int main(int /*argc*/, char** /*argv*/)
{
  auto context = Context::Create({
      .title = "Pangolin Widgets",
  });

  auto widgets = WidgetLayer::Create(
      {.name = "ui.", .size_hint = {Pixels{180}, Parts{1}}});

  auto primitives = DrawnPrimitives::Create({});

  primitives->vertices->queueUpdate(
      std::vector<Eigen::Vector3f>{
          {-2.0f, -2.0f, 0.0f}, {2.0f, -2.0f, 0.0f}, {0.0f, 2.0f, 0.0f}},
      {});

  // TODO: Pangolin Vars will probably get modernized soon...
  Var<double> angle_theta(
      "ui.theta", 0.0, -sophus::kPi<double>, +sophus::kPi<double>);
  Var<bool> filled("ui.filled", true, true);
  Var<double> color_red("ui.red", 0.6, 0.0, 1.0);
  Var<double> color_green("ui.green", 0.3, 0.0, 1.0);
  Var<double> color_blue("ui.blue", 0.4, 0.0, 1.0);

  context->setLayout(widgets | primitives);

  context->loop([&]() {
    primitives->default_color =
        Eigen::Vector4d(color_red, color_green, color_blue, 1.0);

    primitives->element_type = filled ? DrawnPrimitives::Type::triangles
                                      : DrawnPrimitives::Type::line_loop;

    primitives->pose.parent_from_drawable =
        sophus::SE3d(sophus::SO3d::rotZ(angle_theta), Eigen::Vector3d::Zero());

    // We return true to indicate that we should keep running
    return true;
  });

  return 0;
}
