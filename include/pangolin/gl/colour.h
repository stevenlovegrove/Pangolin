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

#include <cmath>

#include <stdexcept>

namespace pangolin
{

/// Represent OpenGL floating point colour: Red, Green and Blue with alpha.
struct Colour
{
    inline static Colour White() {
        return Colour(1.0f,1.0f,1.0f,1.0f);
    }
    inline static Colour Black() {
        return Colour(0.0f,0.0f,0.0f,1.0f);
    }
    inline static Colour Red() {
        return Colour(1.0f,0.0f,0.0f,1.0f);
    }
    inline static Colour Green() {
        return Colour(0.0f,1.0f,0.0f,1.0f);
    }
    inline static Colour Blue() {
        return Colour(0.0f,0.0f,1.0f,1.0f);
    }
    inline static Colour Unspecified() {
        return Colour(
            std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::quiet_NaN(),
            std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::quiet_NaN()
        );
    }

    /// Default constructs white.
    inline Colour()
        : red(1.0f), green(1.0f), blue(1.0f), alpha(1.0f)
    {
    }

    /// Construct from component values
    inline Colour(const float red, const float green, const float blue, const float alpha = 1.0f)
        : red(red), green(green), blue(blue), alpha(alpha)
    {
    }

    /// Construct from rgba array.
    inline Colour(const float rgba[4])
    {
        r = rgba[0];
        g = rgba[1];
        b = rgba[2];
        a = rgba[3];
    }

    /// Return pointer to OpenGL compatible RGBA array.
    inline float* Get()
    {
        return c;
    }

    /// Return this colour with alpha adjusted.
    inline Colour WithAlpha(const float alpha)
    {
        return Colour(r,g,b,alpha);
    }

    /// Construct from HSV Colour
    /// @param hue Colour hue in range [0,1]
    /// @param sat Saturation in range [0,1]
    /// @param val Value / Brightness in range [0,1].
    static inline Colour Hsv(const float hue, const float sat = 1.0f, const float val = 1.0f, const float alpha = 1.0f)
    {
          const float h = 6.0f * hue;
          const int i = (int)floor(h);
          const float f = (i%2 == 0) ? 1-(h-i) : h-i;
          const float m = val * (1-sat);
          const float n = val * (1-sat*f);

          switch(i)
          {
          case 0: return Colour(val,n,m,alpha);
          case 1: return Colour(n,val,m,alpha);
          case 2: return Colour(m,val,n,alpha);
          case 3: return Colour(m,n,val,alpha);
          case 4: return Colour(n,m,val,alpha);
          case 5: return Colour(val,m,n,alpha);
          default:
              throw std::runtime_error("Found extra colour in rainbow.");
          }
    }

    union {
        struct {
            float red;
            float green;
            float blue;
            float alpha;
        };
        struct {
            float r;
            float g;
            float b;
            float a;
        };
        float c[4];
    };

};

/// A ColourWheel is like a continuous colour palate that can be sampled.
/// In the future, different ColourWheels will be supported, but this one
/// is based on sampling hues in HSV colourspace. An indefinite number of
/// unique colours are sampled using the golden angle.
class ColourWheel
{
public:
    /// Construct ColourWheel with Saturation, Value and Alpha constant.
    inline ColourWheel(float saturation = 0.5f, float value = 1.0f, float alpha = 1.0f)
        : unique_colours(0), sat(saturation), val(value), alpha(alpha)
    {

    }

    /// Use Golden ratio (/angle) to pick well spaced colours.
    inline Colour GetColourBin(int i) const
    {
        float hue = i * 0.5f * (3.0f - sqrt(5.0f));
        hue -= (int)hue;
        return Colour::Hsv(hue,sat,val,alpha);
    }

    /// Return next unique colour from ColourWheel.
    inline Colour GetUniqueColour()
    {
        return GetColourBin(unique_colours++);
    }

protected:
    int unique_colours;
    float sat;
    float val;
    float alpha;
};

}
