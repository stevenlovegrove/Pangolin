/* This file is part of the Pangolin Project.
 * http://github.com/stevenlovegrove/Pangolin
 *
 * Copyright (c) 2013 Steven Lovegrove
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

#ifndef PANGOLIN_IMAGE_UTILS_H
#define PANGOLIN_IMAGE_UTILS_H

#include <limits>
#include <utility>

#include <pangolin/image/image.h>
#include <pangolin/plot/range.h>
#include <pangolin/gl/glpixformat.h>

namespace pangolin
{

template<typename T>
std::pair<float,float> GetOffsetScale(const pangolin::Image<T>& img, float type_max, float format_max)
{
    std::pair<float,float> mm(std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());

    for(size_t y=0; y < img.h; ++y) {
        T* pix = (T*)((char*)img.ptr + y*img.pitch);
        for(size_t x=0; x < img.w; ++x) {
            const T val = *(pix++);
            if(val != 0) {
                if(val < mm.first) mm.first = val;
                if(val > mm.second) mm.second = val;
            }
        }
    }

    const float type_scale = format_max / type_max;
    const float offset = -type_scale* mm.first;
    const float scale = type_max / (mm.second - mm.first);
    return std::pair<float,float>(offset, scale);
}

template<typename T>
pangolin::Image<T> ImageRoi( pangolin::Image<T> img, const pangolin::XYRangei& roi )
{
    const int xmin = std::min(roi.x.min,roi.x.max);
    const int ymin = std::min(roi.y.min,roi.y.max);
    return pangolin::Image<T>(
        roi.x.AbsSize(), roi.y.AbsSize(),
        img.pitch, img.RowPtr(ymin) + xmin
    );
}

template<typename T>
std::pair<float,float> GetOffsetScale(const pangolin::Image<T>& img, pangolin::XYRangei iroi, const pangolin::GlPixFormat& glfmt)
{
    iroi.Clamp(0, img.w-1, 0, img.h-1 );

    if(glfmt.gltype == GL_UNSIGNED_BYTE) {
        return GetOffsetScale(ImageRoi(img.template Reinterpret<unsigned char>(), iroi), 255.0f, 1.0f);
    }else if(glfmt.gltype == GL_UNSIGNED_SHORT) {
        return GetOffsetScale(ImageRoi(img.template Reinterpret<unsigned short>(), iroi), 65535.0f, 1.0f);
    }else if(glfmt.gltype == GL_FLOAT) {
        return GetOffsetScale(ImageRoi(img.template Reinterpret<float>(), iroi), 1.0f, 1.0f);
    }else{
        return std::pair<float,float>(0.0f, 1.0f);
    }
}

}

#endif // PANGOLIN_IMAGE_UTILS_H
