// Copyright (c) farm-ng, inc. All rights reserved.

#pragma once

#include <pangolin/gl/gl.h>
#include <pangolin/gl/glformattraits.h>
#include <sophus/image/image.h>
#include <sophus/image/image_view.h>

// Will eventually make it into Pangolin

namespace pangolin {

// https://stackoverflow.com/questions/1198260/how-can-you-iterate-over-the-elements-of-an-stdtuple
template <std::size_t I = 0, typename FuncT, typename... Tp>
inline typename std::enable_if<I == sizeof...(Tp), void>::type for_each(
    std::tuple<Tp...>&, FuncT)  // Unused arguments are given no names.
{}

template <std::size_t I = 0, typename FuncT, typename... Tp>
    inline typename std::enable_if <
    I<sizeof...(Tp), void>::type for_each(std::tuple<Tp...>& t, FuncT f) {
  f(I, std::get<I>(t));
  for_each<I + 1, FuncT, Tp...>(t, f);
}

template <class PixelType>
GlTexture createTextureFromImage(sophus::ImageView<PixelType>& image) {
  using Fmt = GlFormatTraits<PixelType>;
  return pangolin::GlTexture(
      image.width(),
      image.height(),
      Fmt::glinternalformat,
      true,
      0,
      Fmt::glformat,
      Fmt::gltype,
      (void*)image.ptr());
}

template <class PixelType>
GlTexture createTextureForType(sophus::ImageSize dimensions) {
  using Fmt = GlFormatTraits<PixelType>;
  return pangolin::GlTexture(
      dimensions.width,
      dimensions.height,
      Fmt::glinternalformat,
      true,
      0,
      Fmt::glformat,
      Fmt::gltype);
}

template <class... PixelTypes>
void renderToImage(
    std::function<void()> const& user_render_func,
    std::tuple<sophus::MutImageView<PixelTypes>...> output_channels) {
  constexpr size_t kNumChannels = sizeof...(PixelTypes);
  const sophus::ImageSize dim = std::get<0>(output_channels).imageSize();

  pangolin::GlRenderBuffer depth(dim.width, dim.height);
  std::array<pangolin::GlTexture, kNumChannels> channels = {
      createTextureForType<PixelTypes>(dim)...};

  pangolin::GlFramebuffer fbo;
  fbo.AttachDepth(depth);
  for (size_t c = 0; c < kNumChannels; ++c) {
    fbo.AttachColour(channels[c]);
  }
  fbo.Bind();
  glViewport(0, 0, dim.width, dim.height);
  user_render_func();
  fbo.Unbind();

  for_each(output_channels, [&](size_t ch, auto image) {
    using TPix = typename decltype(image)::PixelType;
    using Fmt = GlFormatTraits<TPix>;
    channels[ch].Download(image.ptrMut(), Fmt::glformat, Fmt::gltype);
    PANGO_GL_CHECK();
  });
}

template <class PixelType>
sophus::MutImage<PixelType> renderToImage(
    sophus::ImageSize image_size,
    std::function<void()> const& user_render_func) {
  sophus::MutImage<PixelType> image(image_size);
  renderToImage<PixelType>(user_render_func, {image});
  return image;
}

}  // namespace pangolin
