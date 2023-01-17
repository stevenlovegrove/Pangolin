/* This file is part of the Pangolin Project.
 * http://github.com/stevenlovegrove/Pangolin
 *
 * Copyright (c) 2014 Steven Lovegrove
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

#include <sophus/common/enum.h>

#include <cmath>
#include <limits>
#include <stdexcept>

namespace pangolin
{

SOPHUS_ENUM(
    ColorHue, (red, orange, yellow, chartreuse, green, turquoise, cyan, azure,
               blue, violet, magenta, pink));

SOPHUS_ENUM(ColorBrightness, (bright, medium, dark));
SOPHUS_ENUM(ColorSaturation, (normal, neon));

struct Color {
  constexpr Color(float a = 1.0) : r(1.0), g(1.0), b(1.0), a(a) {}

  constexpr Color(float r, float g, float b, float a = 1.0) :
      r(r), g(g), b(b), a(a)
  {
  }

  /// Construct from HSV Colour
  /// @param hue Color hue in range [0,1]
  /// @param sat Saturation in range [0,1]
  /// @param val Value / Brightness in range [0,1].
  static inline Color Hsv(
      const float hue, const float sat = 1.0f, const float val = 1.0f,
      const float a = 1.0f)
  {
    const float h = 6.0f * hue;
    const int i = (int)floor(h);
    const float f = (i % 2 == 0) ? 1 - (h - i) : h - i;
    const float m = val * (1 - sat);
    const float n = val * (1 - sat * f);

    switch (i) {
      case 0:
        return Color(val, n, m, a);
      case 1:
        return Color(n, val, m, a);
      case 2:
        return Color(m, val, n, a);
      case 3:
        return Color(m, n, val, a);
      case 4:
        return Color(n, m, val, a);
      case 5:
        return Color(val, m, n, a);
      default:
        throw std::runtime_error("Found extra colour in rainbow.");
    }
  }

  static constexpr Color from(
      ColorHue hue, ColorBrightness brightness,
      ColorSaturation sat = ColorSaturation::normal, float a = 1.0)
  {
    float factor = 1.f;
    float zero = 0.f;
    float half = 0.5f;
    switch (brightness) {
      case ColorBrightness::bright: {
        break;
      }
      case ColorBrightness::medium: {
        factor = 0.75f;
        break;
      }
      case ColorBrightness::dark: {
        factor = 0.5f;
        break;
      }
    }
    switch (sat) {
      case ColorSaturation::normal: {
        break;
      }
      case ColorSaturation::neon: {
        zero = 0.5f;
        half = 0.75f;
        break;
      }
    }
    switch (hue) {
      case ColorHue::red: {
        return Color(1.0, zero, zero, a) * factor;
        break;
      }
      case ColorHue::orange: {
        return Color(1.0, half, zero, a) * factor;
        break;
      }
      case ColorHue::yellow: {
        return Color(1.0, 1.0, zero, a) * factor;
        break;
      }
      case ColorHue::chartreuse: {
        return Color(half, 1.0, zero, a) * factor;
        break;
      }
      case ColorHue::green: {
        return Color(zero, 1.0, zero, a) * factor;
        break;
      }
      case ColorHue::turquoise: {
        return Color(zero, 1.0, half, a) * factor;
        break;
      }
      case ColorHue::cyan: {
        return Color(zero, 1.0, 1.0, a) * factor;
        break;
      }
      case ColorHue::azure: {
        return Color(zero, half, 1.0, a) * factor;
        break;
      }
      case ColorHue::blue: {
        return Color(zero, zero, 1.0, a) * factor;
        break;
      }
      case ColorHue::violet: {
        return Color(half, zero, 1.0, a) * factor;
        break;
      }
      case ColorHue::magenta: {
        return Color(1.0, zero, 1.0, a) * factor;
        break;
      }
      case ColorHue::pink: {
        return Color(1.0, zero, half, a) * factor;
        break;
      }
    }
    FARM_IMPL_ABORT();
  }

  // bright colors
  static constexpr Color brightRed(float a = 1.0)
  {
    return Color::from(
        ColorHue::red, ColorBrightness::bright, ColorSaturation::normal, a);
  }
  static constexpr Color brightGreen(float a = 1.0)
  {
    return Color::from(
        ColorHue::green, ColorBrightness::bright, ColorSaturation::normal, a);
  }
  static constexpr Color brightBlue(float a = 1.0)
  {
    return Color::from(
        ColorHue::blue, ColorBrightness::bright, ColorSaturation::normal, a);
  }
  static constexpr Color brightYellow(float a = 1.0)
  {
    return Color::from(
        ColorHue::yellow, ColorBrightness::bright, ColorSaturation::normal, a);
  }
  static constexpr Color brightMagenta(float a = 1.0)
  {
    return Color::from(
        ColorHue::magenta, ColorBrightness::bright, ColorSaturation::normal, a);
  }
  static constexpr Color brightCyan(float a = 1.0)
  {
    return Color::from(
        ColorHue::cyan, ColorBrightness::bright, ColorSaturation::normal, a);
  }
  static constexpr Color brightOrange(float a = 1.0)
  {
    return Color::from(
        ColorHue::orange, ColorBrightness::bright, ColorSaturation::normal, a);
  }
  static constexpr Color brightTurquoise(float a = 1.0)
  {
    return Color::from(
        ColorHue::turquoise, ColorBrightness::bright, ColorSaturation::normal,
        a);
  }
  static constexpr Color brightViolet(float a = 1.0)
  {
    return Color::from(
        ColorHue::violet, ColorBrightness::bright, ColorSaturation::normal, a);
  }
  static constexpr Color brightPink(float a = 1.0)
  {
    return Color::from(
        ColorHue::pink, ColorBrightness::bright, ColorSaturation::normal, a);
  }
  static constexpr Color brightChartreuse(float a = 1.0)
  {
    return Color::from(
        ColorHue::chartreuse, ColorBrightness::bright, ColorSaturation::normal,
        a);
  }
  static constexpr Color brightAzure(float a = 1.0)
  {
    return Color::from(
        ColorHue::azure, ColorBrightness::bright, ColorSaturation::normal, a);
  }

  // colors
  static constexpr Color red(float a = 1.0)
  {
    return Color::from(
        ColorHue::red, ColorBrightness::medium, ColorSaturation::normal, a);
  }
  static constexpr Color green(float a = 1.0)
  {
    return Color::from(
        ColorHue::green, ColorBrightness::medium, ColorSaturation::normal, a);
  }
  static constexpr Color blue(float a = 1.0)
  {
    return Color::from(
        ColorHue::blue, ColorBrightness::medium, ColorSaturation::normal, a);
  }
  static constexpr Color yellow(float a = 1.0)
  {
    return Color::from(
        ColorHue::yellow, ColorBrightness::medium, ColorSaturation::normal, a);
  }
  static constexpr Color magenta(float a = 1.0)
  {
    return Color::from(
        ColorHue::magenta, ColorBrightness::medium, ColorSaturation::normal, a);
  }
  static constexpr Color cyan(float a = 1.0)
  {
    return Color::from(
        ColorHue::cyan, ColorBrightness::medium, ColorSaturation::normal, a);
  }
  static constexpr Color orange(float a = 1.0)
  {
    return Color::from(
        ColorHue::orange, ColorBrightness::medium, ColorSaturation::normal, a);
  }
  static constexpr Color turquoise(float a = 1.0)
  {
    return Color::from(
        ColorHue::turquoise, ColorBrightness::medium, ColorSaturation::normal,
        a);
  }
  static constexpr Color violet(float a = 1.0)
  {
    return Color::from(
        ColorHue::violet, ColorBrightness::medium, ColorSaturation::normal, a);
  }
  static constexpr Color pink(float a = 1.0)
  {
    return Color::from(
        ColorHue::pink, ColorBrightness::medium, ColorSaturation::normal, a);
  }
  static constexpr Color chartreuse(float a = 1.0)
  {
    return Color::from(
        ColorHue::chartreuse, ColorBrightness::medium, ColorSaturation::normal,
        a);
  }
  static constexpr Color azure(float a = 1.0)
  {
    return Color::from(
        ColorHue::azure, ColorBrightness::medium, ColorSaturation::normal, a);
  }
  // dark colors
  static constexpr Color darkRed(float a = 1.0)
  {
    return Color::from(
        ColorHue::red, ColorBrightness::dark, ColorSaturation::normal, a);
  }
  static constexpr Color darkGreen(float a = 1.0)
  {
    return Color::from(
        ColorHue::green, ColorBrightness::dark, ColorSaturation::normal, a);
  }
  static constexpr Color darkBlue(float a = 1.0)
  {
    return Color::from(
        ColorHue::blue, ColorBrightness::dark, ColorSaturation::normal, a);
  }
  static constexpr Color darkYellow(float a = 1.0)
  {
    return Color::from(
        ColorHue::yellow, ColorBrightness::dark, ColorSaturation::normal, a);
  }
  static constexpr Color darkMagenta(float a = 1.0)
  {
    return Color::from(
        ColorHue::magenta, ColorBrightness::dark, ColorSaturation::normal, a);
  }
  static constexpr Color darkCyan(float a = 1.0)
  {
    return Color::from(
        ColorHue::cyan, ColorBrightness::dark, ColorSaturation::normal, a);
  }
  static constexpr Color darkOrange(float a = 1.0)
  {
    return Color::from(
        ColorHue::orange, ColorBrightness::dark, ColorSaturation::normal, a);
  }
  static constexpr Color darkTurquoise(float a = 1.0)
  {
    return Color::from(
        ColorHue::turquoise, ColorBrightness::dark, ColorSaturation::normal, a);
  }
  static constexpr Color darkViolet(float a = 1.0)
  {
    return Color::from(
        ColorHue::violet, ColorBrightness::dark, ColorSaturation::normal, a);
  }
  static constexpr Color darkPink(float a = 1.0)
  {
    return Color::from(
        ColorHue::pink, ColorBrightness::dark, ColorSaturation::normal, a);
  }
  static constexpr Color darkChartreuse(float a = 1.0)
  {
    return Color::from(
        ColorHue::chartreuse, ColorBrightness::dark, ColorSaturation::normal,
        a);
  }
  static constexpr Color darkAzure(float a = 1.0)
  {
    return Color::from(
        ColorHue::azure, ColorBrightness::dark, ColorSaturation::normal, a);
  }

  // neon colors
  static constexpr Color neonRed(float a = 1.0)
  {
    return Color::from(
        ColorHue::red, ColorBrightness::bright, ColorSaturation::neon, a);
  }
  static constexpr Color neonGreen(float a = 1.0)
  {
    return Color::from(
        ColorHue::green, ColorBrightness::bright, ColorSaturation::neon, a);
  }
  static constexpr Color neonBlue(float a = 1.0)
  {
    return Color::from(
        ColorHue::blue, ColorBrightness::bright, ColorSaturation::neon, a);
  }
  static constexpr Color neonYellow(float a = 1.0)
  {
    return Color::from(
        ColorHue::yellow, ColorBrightness::bright, ColorSaturation::neon, a);
  }
  static constexpr Color neonMagenta(float a = 1.0)
  {
    return Color::from(
        ColorHue::magenta, ColorBrightness::bright, ColorSaturation::neon, a);
  }
  static constexpr Color neonCyan(float a = 1.0)
  {
    return Color::from(
        ColorHue::cyan, ColorBrightness::bright, ColorSaturation::neon, a);
  }
  static constexpr Color neonOrange(float a = 1.0)
  {
    return Color::from(
        ColorHue::orange, ColorBrightness::bright, ColorSaturation::neon, a);
  }
  static constexpr Color neonTurquoise(float a = 1.0)
  {
    return Color::from(
        ColorHue::turquoise, ColorBrightness::bright, ColorSaturation::neon, a);
  }
  static constexpr Color neonViolet(float a = 1.0)
  {
    return Color::from(
        ColorHue::violet, ColorBrightness::bright, ColorSaturation::neon, a);
  }
  static constexpr Color neonPink(float a = 1.0)
  {
    return Color::from(
        ColorHue::pink, ColorBrightness::bright, ColorSaturation::neon, a);
  }
  static constexpr Color neonChartreuse(float a = 1.0)
  {
    return Color::from(
        ColorHue::chartreuse, ColorBrightness::bright, ColorSaturation::neon,
        a);
  }
  static constexpr Color neonAzure(float a = 1.0)
  {
    return Color::from(
        ColorHue::azure, ColorBrightness::bright, ColorSaturation::neon, a);
  }

  // shades of gray
  static constexpr Color black(float a = 1.0)
  {
    return Color(0.0, 0.0, 0.0, a);
  }
  static constexpr Color white(float a = 1.0)
  {
    return Color(1.0, 1.0, 1.0, a);
  }
  static constexpr Color brightGray(float a = 1.0)
  {
    return Color(0.75, 0.75, 0.75, a);
  }
  static constexpr Color darkGray(float a = 1.0)
  {
    return Color(0.25, 0.25, 0.25, a);
  }
  static constexpr Color gray(float a = 1.0) { return Color(0.5, 0.5, 0.5, a); }

  constexpr Color operator*(double scale)
  {
    return Color(scale * r, scale * g, scale * b, a);
  }

  union {
    struct {
      float r;
      float g;
      float b;
      float a;
    };
    float c[4];
  };

  //   float r;
  //   float g;
  //   float b;
  //   float a;
};

static_assert(sizeof(Color) == 4 * sizeof(float));

/// A ColorWheel is like a continuous color palate that can be sampled.
/// In the future, different ColorWheels will be supported, but this one
/// is based on sampling hues in HSV colorspace. An indefinite number of
/// unique colours are sampled using the golden angle.
class ColorWheel
{
  public:
  /// Use Golden ratio (/angle) to pick well spaced colours.
  inline static Color GoldenColor(
      int index, float saturation = 0.5f, float value = 1.0f, float alpha = 1.0)
  {
    float hue = index * 0.5f * (3.0f - sqrt(5.0f));
    hue = hue - floor(hue);
    return Color::Hsv(hue, saturation, value, alpha);
  }

  /// Construct ColourWheel with Saturation, Value and Alpha constant.
  inline ColorWheel(
      float saturation = 0.5f, float value = 1.0f, float a = 1.0f) :
      unique_colors(0), sat(saturation), val(value), a(a)
  {
  }

  /// Use Golden ratio (/angle) to pick well spaced colours.
  inline Color GetColorBin(int i) const { return GoldenColor(i, sat, val, a); }

  /// Return next unique colour from ColourWheel.
  inline Color GetUniqueColor() { return GetColorBin(unique_colors++); }

  /// Reset colour wheel counter to initial state
  inline void Reset() { unique_colors = 0; }

  inline int GetCurrentIndex() const { return unique_colors; }

  protected:
  int unique_colors;
  float sat;
  float val;
  float a;
};

}  // namespace pangolin
