#include <pangolin/context/depth_sampler_image.h>
#include <pangolin/utils/logging.h>
#include <pangolin/utils/shared.h>

namespace pangolin
{

struct DepthSamplerImageImpl : public DepthSamplerImage {
  DepthSamplerImageImpl(const Params& p) :
      kind_(p.kind), depth_image_(p.depth_image)
  {
    if (kind_ != DepthKind::zaxis) {
      PANGO_UNIMPLEMENTED();
    }
  }

  void setDepthImage(const sophus::Image<float>& image) override
  {
    depth_image_ = image;
  }

  std::optional<Sample> sampleDepth(
      const SampleLocation& location, int patch_rad, sophus::RegionF64 near_far,
      const Context* default_context) override
  {
    const Eigen::Array2i pix = location.pos_camera_pixel.cast<int>();

    if (!depth_image_.isEmpty()) {
      if (depth_image_.imageSize().contains(pix, patch_rad)) {
        Sample sample{.depth_kind = DepthKind::zaxis};
        auto patch = depth_image_.subview(
            pix - patch_rad, sophus::ImageSize(2 * patch_rad, 2 * patch_rad));
        patch.visit([&](float real_z) {
          if (real_z > 0) sample.min_max.extend(real_z);
        });
        if (!sample.min_max.isEmpty()) {
          return sample;
        }
      }
    }

    return std::nullopt;
  }

  DepthKind kind_;
  sophus::Image<float> depth_image_;
};

Shared<DepthSamplerImage> DepthSamplerImage::Create(DepthSamplerImage::Params p)
{
  return Shared<DepthSamplerImageImpl>::make(p);
}

}  // namespace pangolin
