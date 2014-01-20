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
        : rmin(rmin), rmax(rmax)
    {
    }

    Range& operator+=(float v)
    {
        rmin += v;
        rmax += v;
        return *this;
    }

    Range& operator-=(float v)
    {
        rmin -= v;
        rmax -= v;
        return *this;
    }

    Range& operator+=(const Range& o)
    {
        rmin += o.rmin;
        rmax += o.rmax;
        return *this;
    }

    Range& operator-=(const Range& o)
    {
        rmin -= o.rmin;
        rmax -= o.rmax;
        return *this;
    }

    Range operator-(const Range& o) const
    {
        return Range(rmin - o.rmin, rmax - o.rmax);
    }

    Range operator*(float s) const
    {
        return Range(s*rmin, s*rmax);
    }

    float Size() const
    {
        return rmax - rmin;
    }

    float Mid() const
    {
        return (rmin + rmax) / 2.0f;
    }

    void Scale(float s, float center = 0.0f)
    {
        rmin = s*(rmin-center) + center;
        rmax = s*(rmax-center) + center;
    }

    float rmin;
    float rmax;
};

struct XYRange
{
    XYRange(const Range& xrange, const Range& yrange)
        : xrange(xrange), yrange(yrange)
    {
    }

    XYRange(float xmin, float xmax, float ymin, float ymax)
        : xrange(xmin,xmax), yrange(ymin,ymax)
    {
    }

    XYRange operator-(const XYRange& o) const
    {
        return XYRange(xrange - o.xrange, yrange - o.yrange);
    }

    XYRange operator*(float s) const
    {
        return XYRange(xrange*s, yrange*s);
    }

    XYRange& operator+=(const XYRange& o)
    {
        xrange += o.xrange;
        yrange += o.yrange;
        return *this;
    }

    void Scale(float sx, float sy, float centerx, float centery)
    {
        xrange.Scale(sx, centerx);
        yrange.Scale(sy, centery);
    }

    float Area() const
    {
        return xrange.Size() * yrange.Size();
    }

    Range xrange;
    Range yrange;
};

}

#endif //PANGOLIN_RANGE_H
