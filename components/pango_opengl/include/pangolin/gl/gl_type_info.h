#pragma once

#include "farm_ng/core/logging/expected.h"

#include <pangolin/gl/glplatform.h>
#include <sophus/image/dyn_image_types.h>

namespace pangolin
{

namespace
{
template <typename T>
bool between(const T& val, const T& low_inclusive, const T& high_inclusive)
{
  return low_inclusive <= val && val <= high_inclusive;
}

template <typename T>
bool isOneOf(const T& val, const std::initializer_list<T>& of)
{
  for (auto x : of) {
    if (val == x) return true;
  }
  return false;
}
}  // namespace

// Struct representing a sized form and the corresponding
// base format and component type
// https://registry.khronos.org/OpenGL-Refpages/gl4/html/glTexImage2D.xhtml
struct GlFormatInfo {
  // e.g. GL_R8, GL_R16, ..., GL_RGBA32F
  GLint gl_sized_format;

  // e.g. GL_RED, GL_RG, GL_RGB, GL_RGBA
  GLint gl_base_format;

  // GL_BYTE, GL_UNSIGNED_SHORT, ...
  GLint gl_type;
};

inline farm_ng::Expected<GlFormatInfo> glTypeInfo(
    const sophus::PixelFormat& pixel_type)
{
  constexpr static GLint type_table[] = {
      0,  // unspecified
      GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT, GL_FLOAT};

  // inner stride is GL_RED, GL_RG, GL_RGB, GL_RGBA
  // outer stride is format type
  constexpr static GLint format_table[][4] = {
      {GL_RED, GL_RG, GL_RGB, GL_RGBA},                // base (untyped)
      {GL_R8, GL_RG8, GL_RGB8, GL_RGBA8},              // uint8_t
      {GL_R16, GL_RG16, GL_RGB16, GL_RGBA16},          // uint16_t
      {GL_R32UI, GL_RG32UI, GL_RGB32UI, GL_RGBA32UI},  // uint32_t
      {GL_R32F, GL_RG32F, GL_RGB32F, GL_RGBA32F},      // float (32bits)
  };

  // Make sure we'll be in bounds...
  const int nbytes = pixel_type.num_bytes_per_pixel_channel;
  const bool bfixed = pixel_type.number_type == sophus::NumberType::fixed_point;

  if (between(pixel_type.num_channels, 1, 4) && isOneOf(nbytes, {1, 2, 4})) {
    const int cidx = pixel_type.num_channels - 1;
    const int fidx = (nbytes == 4 && bfixed) ? 3 : nbytes;
    return GlFormatInfo(
        {.gl_sized_format = format_table[fidx][cidx],
         .gl_base_format = format_table[0][cidx],
         .gl_type = type_table[fidx]});
  }

  return SOPHUS_UNEXPECTED("Unsupported GL image type, {}", pixel_type);
}

}  // namespace pangolin
