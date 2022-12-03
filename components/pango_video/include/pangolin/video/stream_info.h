/* This file is part of the Pangolin Project.
 * http://github.com/stevenlovegrove/Pangolin
 *
 * Copyright (c) 2011 Steven Lovegrove
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

#include <pangolin/image/image.h>
#include <pangolin/image/runtime_image.h>
#include <pangolin/image/pixel_format.h>

namespace pangolin {

class PANGOLIN_EXPORT StreamInfo
{
public:
    inline StreamInfo(): shape_(0,0,0), offset_bytes_(0) {}

    inline StreamInfo(RuntimePixelType fmt, const sophus::ImageShape shape, size_t offset_bytes )
        : fmt_(fmt), shape_(shape), offset_bytes_(offset_bytes) {}

    inline sophus::ImageShape shape() const
    {
        return shape_;
    }

    inline RuntimePixelType format() const
    {
        return fmt_;
    }

    inline size_t offsetBytes() const
    {
        return offset_bytes_;
    }

    inline size_t rowBytes() const
    {
        return format().bytesPerPixel() * shape().width();
    }

    //! Return Image wrapper around raw base pointer
    inline sophus::ImageView<uint8_t> StreamImage(const uint8_t* base_ptr) const {
        return { shape_, base_ptr + offset_bytes_ };
    }

    inline sophus::MutImageView<uint8_t> StreamImage(uint8_t* base_ptr) const {
        return { shape_, base_ptr + offset_bytes_ };
    }

    inline IntensityImage<> copyToRuntimeImage(const uint8_t* base_ptr) const {
        PANGO_DEBUG("Unneeded image copy happening...");
        IntensityImage<> runtime(shape_, fmt_);
        sophus::details::pitchedCopy(
            const_cast<uint8_t*>(runtime.rawPtr()),
            runtime.shape().pitchBytes(),
            base_ptr, shape_.pitchBytes(), shape_.imageSize(),
            fmt_.bytesPerPixel()
        );
        return runtime;
    }

protected:
    RuntimePixelType fmt_;
    sophus::ImageShape shape_;
    size_t offset_bytes_;
};

}
