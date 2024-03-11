#include <pangolin/context/context.h>
#include <pangolin/context/depth_sampler.h>
#include <pangolin/gl/gl_depth.h>
#include <pangolin/gl/glplatform.h>
#include <pangolin/utils/shared.h>
#include <pangolin/utils/variant_overload.h>

namespace pangolin
{

class DepthSamplerImpl : public DepthSampler
{
  public:
  DepthSamplerImpl(const DepthSampler::Params& p) : context_(p.context) {}

  std::optional<Sample> sampleDepth(
      const SampleLocation& location, int patch_rad, sophus2::RegionF64 near_far,
      const Context* default_context) override
  {
    using namespace sophus2;

    const Eigen::Array2i pix = location.pos_window.cast<int>();
    auto region = Region2I::fromMinMax(
        pix - Eigen::Array2i(patch_rad, patch_rad),
        pix + Eigen::Array2i(patch_rad, patch_rad));

    const Context* context = context_ ? context_.get() : default_context;
    PANGO_CHECK(context);
    if (!context) return std::nullopt;

    const auto patch = context->read(region, Context::Attachment::depth);
    Sample sample{.depth_kind = DepthKind::zaxis};

    visitImage(
        overload{
            [&](const ImageView<float>& image) {
              image.visit([&](float gl_z) {
                float real_z =
                    realDepthFromGl(gl_z, near_far.min(), near_far.max());
                if (near_far.min() < real_z && real_z < near_far.max()) {
                  sample.min_max.extend(real_z);
                }
              });
            },
            [&](const auto& image) { PANGO_UNREACHABLE(); },
        },
        patch);

    if (sample.min_max.isEmpty()) {
      return std::nullopt;
    }
    return sample;
  }

  std::shared_ptr<Context> context_;
};

Shared<DepthSampler> DepthSampler::Create(DepthSampler::Params p)
{
  return Shared<DepthSamplerImpl>::make(p);
}

}  // namespace pangolin
