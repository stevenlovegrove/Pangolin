#include <pangolin/context/context.h>
#include <pangolin/layer/all_layers.h>
#include <pangolin/video/video.h>

/*
  == Pangolin-by-example ==

*/

using namespace pangolin;
using namespace sophus;

int main(int argc, char** argv)
{
  PANGO_ENSURE(argc == 2, "Please provide one argument - the URL to a video.");
  const std::string video_url = argv[1];

  auto context = Context::Create({
      .title = "Pangolin Video",
      .window_size = {640, 480},
  });

  auto video_input = OpenVideo(video_url);

  auto video_view = DrawnImage::Create({});

  context->setLayout(video_view);

  context->loop([&]() {
    if (auto maybe_image = optGet(video_input->GrabImages(), 0)) {
      video_view->image->update(*maybe_image);
    }
    return true;
  });
  return 0;
}
