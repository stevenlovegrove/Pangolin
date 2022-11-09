#include <pangolin/context/context.h>
#include <pangolin/video/video.h>

/*
  == Pangolin-by-example ==

*/

using namespace pangolin;
using namespace sophus;

int main( int /*argc*/, char** /*argv*/ )
{
    auto context = Context::Create({
        .title="Pangolin Video",
        .window_size = {640,480},
    } );

    auto video_input = OpenVideo("test:[size=640x480]//");
    auto video_view = DrawnImage::Create({});

    context->setLayout( video_view );

    context->loop([&](){
        if( auto maybe_image = optGet(video_input->GrabImages(), 0) ) {
            video_view->image->update(*maybe_image);
        }
        return true;
    });
    return 0;
}
