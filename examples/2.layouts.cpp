#include <pangolin/context/context.h>
#include <pangolin/gui/all_layers.h>
#include <pangolin/image/image_io.h>

/*
  == Pangolin-by-example ==

  Pangolin let's you easily compose different window elements on the screen
  through simple operator overloading to specify their relative positioning.
  We use the following binary operators to specify the layout:
    |   horizontal layout. Elements are arranged left-to-right
    /   vertical layout. Elements are arranged top-to-bottom
    ,   tabbed layout. Elements are tabbed and share a region.
    ^   stacked layout. Elements are stacked top-to-bottom for overlay
   ( )  grouping

   For example, (a | b | c) / d would represent three horizontal elements
   a,b and c size-by-size all sat vertically above of a long element d

   This program:
   * Creates a window with a default size and title
   * Loads two images from the internet
   * Creates a 'DrawnPrimitive' - an object that holds vertex data - and
     we will fill it with 3 vertices to form a triangle
   * Adds the two images and DrawnPrimitive to the window in a simple layout

   Note:
   * Pangolin uses a factory pattern for creating most non-trivial objects.
     This pattern is pimpl-like without the pain - it hides the implementation
     to let us swap out different platforms or graphics libraries and it has
     the added benefit of speeding up compilation since fewer details are leeked
     in the header includes.
   * Pangolin also makes wide use of argument structs for constructors to allow
     for easy parameter defaulting and by-name argument referencing. This lets
     us provide sensible defaults for nearly everything, but still allows for
     fine grain control when needed.
   * The host vertex array here as an r-value is *moved* into an update queue
     which can transfer the data to the graphics device asynchronously. Call
     sync() to guarentee the data is ready (Pangolin will do that for you with
     the default renderer).

*/

using namespace pangolin;

char const* image_url_1 =
    "https://www.wwf.org.uk/sites/default/files/styles/gallery_image/public/"
    "2019-09/pangolin_with_tongue_out.jpg";
char const* image_url_2 =
    "https://upload.wikimedia.org/wikipedia/commons/thumb/2/23/"
    "Space_Needle_2011-07-04.jpg/500px-Space_Needle_2011-07-04.jpg";

int main(int /*argc*/, char** /*argv*/)
{
  auto context = Context::Create({
      .title = "Pangolin Layouts",
      .window_size = {1024, 600},
  });

  auto im1 = LoadImage(image_url_1);
  auto im2 = LoadImage(image_url_2);

  // Create an object to render a triangle
  auto primitives = DrawnPrimitives::Create(
      {.element_type = DrawnPrimitives::Type::triangles,
       .default_color = {1.0f, 0.0f, 0.0f, 1.0f}});

  primitives->vertices->update(
      std::vector<Eigen::Vector3f>{
          {-1.0f, -1.0f, 0.0f}, {1.0f, -1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
      {});

  // Add the images and the triangle to the window.
  // Pangolin automatically adds some appropriate 'Layers' to
  // enable interaction with these elements.
  context->setLayout((im1 / primitives) | im2);
  context->loop();
  return 0;
}
