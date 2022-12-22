/* This file is part of the Pangolin Project.
 * http://github.com/stevenlovegrove/Pangolin
 *
 * Copyright (c) 2015 Steven Lovegrove
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#pragma once

#include <pangolin/gl/gltext.h>

#include <cstdarg>
#include <cstdio>
#include <unordered_map>

namespace pangolin
{

class PANGOLIN_EXPORT GlFont
{
  public:
  enum class FontBitmapType { Alpha = 0, SDF, MSDF };

  // Load GL Font data. Delay uploading as texture until first use.
  GlFont(
      const unsigned char* ttf_buffer, float pixel_height, int tex_w = 1024,
      int tex_h = 1024, bool use_alpha_font = true);
  GlFont(
      const std::string& filename, float pixel_height, int tex_w = 1024,
      int tex_h = 1024, bool use_alpha_font = true);
  GlFont(const std::string& atlas_filename, const std::string& json_filename);

  virtual ~GlFont();

  // Generate renderable GlText object from this font.
  GlText Text(const char* fmt, ...);

  // Utf8 encoded string
  GlText Text(const std::string& utf8);

  inline float Height() const { return font_height_px; }
  inline float MaxWidth() const { return font_max_width_px; }

  // protected:
  sophus::Image<Eigen::Vector4f> MakeFontLookupImage();
  sophus::Image<uint16_t> MakeFontIndexImage(const std::string& utf8);
  std::u16string to_index_string(const std::u32string& utf32);
  std::u16string to_index_string(const std::string& utf8);

  void InitialiseFont(
      const unsigned char* ttf_buffer, float pixel_height, int tex_w, int tex_h,
      bool use_alpha_font);
  void InitialiseFontFromAtlas(
      const std::string& atlas_bitmap, const std::string& atlas_json);

  // This can only be called once GL context is initialised
  void InitialiseGlTexture();

  float font_height_px;
  float font_max_width_px;
  float default_advance_px;

  FontBitmapType bitmap_type;
  std::array<float, 2> bitmap_max_sdf_dist_uv;

  IntensityImage<> font_bitmap;
  GlTexture mTex;
  bool use_alpha_font;

  using codepoint_t = uint32_t;
  using codepointpair_t = std::pair<codepoint_t, codepoint_t>;

  std::map<codepoint_t, GlChar> chardata;
  std::map<codepointpair_t, GLfloat> kern_table;
};

std::shared_ptr<GlFont> build_builtin_font(
    float pixel_height, int tex_w = 1024, int tex_h = 1024,
    bool use_alpha_font = true);

}  // namespace pangolin
