#include <pangolin/context/context.h>
#include <pangolin/gui/all_layers.h>
#include <pangolin/image/image_io.h>

/*
  == Pangolin-by-example ==

  Pangolin sits ontop of a graphics accelerated render library (such as OpenGL)
  and provides some layout, visualization and plotting utilities, along with
  image and video loading tools. It's goal is to minimize boilerplate and help
  you make and iterate on technical applications fast.

  Pangolin is particularly targetted to help create Computer Vision applications
  where it has built in support for rendering within non-linear camera models
  and combining 2d and 3d rendering for augmented visualizations.

  This sample demonstrates windowing, layout and image loading to show how tight
  integration of these components in one library can help make things very
  simple.

  This program:
  * Creates a new graphics context with a graphical window.
  * Loads an image from the internet
  * Adds the image to the window.
  * Yields to the contexts processing loop which handles rendering and window
    events
  * It will exit the loop when the user closes the window.

  Implicitly:
  * Creating input handlers that allow a user to inspect the image and zoom /
    pan around.
  * Deals with HDR images and allows for real-time normalization of the range
    (TODO)
*/


using namespace pangolin;

const char* image_url = "https://www.wwf.org.uk/sites/default/files/styles/gallery_image/public/2019-09/pangolin_with_tongue_out.jpg";

int main( int /*argc*/, char** /*argv*/ )
{
    auto context = Context::Create({ .title="Hello Pangolin World!", } );
    context->setLayout(LoadImage(image_url));
    context->loop();
    return 0;
}
