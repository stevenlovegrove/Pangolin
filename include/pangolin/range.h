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

#ifndef PANGOLIN_RANGE_H
#define PANGOLIN_RANGE_H

namespace pangolin
{

struct Range
{
    Range(float rmin, float rmax)
        : min(rmin), max(rmax)
    {
    }

    Range& operator+=(float v)
    {
        min += v;
        max += v;
        return *this;
    }

    Range& operator-=(float v)
    {
        min -= v;
        max -= v;
        return *this;
    }

    Range& operator*=(float v)
    {
        min *= v;
        max *= v;
        return *this;
    }

    Range& operator/=(float v)
    {
        min /= v;
        max /= v;
        return *this;
    }

    Range& operator+=(const Range& o)
    {
        min += o.min;
        max += o.max;
        return *this;
    }

    Range& operator-=(const Range& o)
    {
        min -= o.min;
        max -= o.max;
        return *this;
    }

    Range operator+(const Range& o) const
    {
        return Range(min + o.min, max + o.max);
    }

    Range operator-(const Range& o) const
    {
        return Range(min - o.min, max - o.max);
    }

    Range operator*(float s) const
    {
        return Range(s*min, s*max);
    }

    float Size() const
    {
        return max - min;
    }

    float Mid() const
    {
        return (min + max) / 2.0f;
    }

    void Scale(float s, float center = 0.0f)
    {
        min = s*(min-center) + center;
        max = s*(max-center) + center;
    }

    float min;
    float max;
};

struct XYRange
{
    XYRange(const Range& xrange, const Range& yrange)
        : x(xrange), y(yrange)
    {
    }

    XYRange(float xmin, float xmax, float ymin, float ymax)
        : x(xmin,xmax), y(ymin,ymax)
    {
    }

    XYRange operator-(const XYRange& o) const
    {
        return XYRange(x - o.x, y - o.y);
    }

    XYRange operator*(float s) const
    {
        return XYRange(x*s, y*s);
    }

    XYRange& operator+=(const XYRange& o)
    {
        x += o.x;
        y += o.y;
        return *this;
    }

    void Scale(float sx, float sy, float centerx, float centery)
    {
        x.Scale(sx, centerx);
        y.Scale(sy, centery);
    }

    float Area() const
    {
        return x.Size() * y.Size();
    }

    Range x;
    Range y;
};

}

#endif //PANGOLIN_RANGE_H
