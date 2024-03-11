#include <pangolin/context/context.h>
#include <pangolin/layer/all_layers.h>

/*
  == Pangolin-by-example ==

*/

using namespace pangolin;
using namespace sophus2;

std::random_device rd;
std::mt19937 e2(rd());
std::uniform_real_distribution<> dist(0.0, 1.0);

sophus2::Image<sophus2::Pixel3F32> testImage(int w, int h)
{
  const sophus2::Pixel3F32 mean =
      sophus2::Pixel3F32(dist(e2), dist(e2), dist(e2));
  sophus2::MutImage<sophus2::Pixel3F32> img(ImageSize(w, h));
  img.mutate([&](sophus2::Pixel3F32&) -> sophus2::Pixel3F32 {
    const sophus2::Pixel3F32 r(dist(e2), dist(e2), dist(e2));
    return (mean + r / 10.0);
  });
  return img;
}

int main(int argc, char** argv)
{
  const int w = 8;
  const int h = 5;
  const int win_scale = 100;
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
