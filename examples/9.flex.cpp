#include <pangolin/context/context.h>
#include <pangolin/gui/all_layers.h>

/*
  == Pangolin-by-example ==

*/

using namespace pangolin;
using namespace sophus;

std::random_device rd;
std::mt19937 e2(rd());
std::uniform_real_distribution<> dist(0.0, 1.0);

sophus::Image<sophus::Pixel3F32> testImage(int w, int h)
{
  const sophus::Pixel3F32 mean =
      sophus::Pixel3F32(dist(e2), dist(e2), dist(e2));
  sophus::MutImage<sophus::Pixel3F32> img(ImageSize(w, h));
  img.mutate([&](sophus::Pixel3F32&) -> sophus::Pixel3F32 {
    const sophus::Pixel3F32 r(dist(e2), dist(e2), dist(e2));
    return (mean + r / 10.0);
  });
  return img;
}

int main(int argc, char** argv)
{
  int const w = 8;
  int const h = 5;
  int const win_scale = 100;
  auto context = Context::Create({
      .title = "Pangolin Video",
      .window_size = {win_scale * w, win_scale * h},
  });

  auto layout = flex(
      testImage(w + 2, h), testImage(w + 1, h), testImage(w - 1, h),
      testImage(w + 3, h), testImage(w + 2, h), testImage(w - 1, h),
      testImage(w + 3, h), testImage(w + 2, h), testImage(w - 1, h),
      testImage(w + 3, h), testImage(w + 2, h), testImage(w - 1, h),
      testImage(w + 3, h), testImage(w + 2, h), testImage(w - 1, h),
      testImage(w + 3, h), testImage(w + 2, h), testImage(w, h));

  context->setLayout(layout);

  context->loop();
  return 0;
}
