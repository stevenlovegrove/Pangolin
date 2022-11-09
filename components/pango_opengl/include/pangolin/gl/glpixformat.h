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

#include <pangolin/gl/glplatform.h>
#include <pangolin/gl/glformattraits.h>
#include <pangolin/image/pixel_format.h>
#include <stdexcept>

namespace pangolin {

// This class may dissapear in the future
struct GlPixFormat
{
    GlPixFormat() {}

    GlPixFormat(const RuntimePixelType& fmt)
    {
        switch( fmt.num_channels) {
        case 1: glformat = GL_RED; break;
        case 2: glformat = GL_RG; break;
        case 3: glformat = GL_RGB;  break;
        case 4: glformat = GL_RGBA; break;
        default: throw std::runtime_error("Unable to form OpenGL format from video format: '" + std::string(ToString(fmt)) + "'.");
        }

        const bool is_integral = fmt.number_type == sophus::NumberType::fixed_point;

        switch (fmt.num_bytes_per_pixel_channel) {
        case 1: gltype = GL_UNSIGNED_BYTE; break;
        // case 10: gltype = GL_UNSIGNED_SHORT; break;
        // case 12: gltype = GL_UNSIGNED_SHORT; break;
        case 2: gltype = GL_UNSIGNED_SHORT; break;
        case 4: gltype = (is_integral ? GL_UNSIGNED_INT : GL_FLOAT); break;
        case 8: gltype = (is_integral ? GL_UNSIGNED_INT64_NV : GL_DOUBLE); break;
        default: throw std::runtime_error("Unknown OpenGL data type for video format: '" + std::string(ToString(fmt)) + "'.");
        }

        if(glformat == GL_RED) {
            if(gltype == GL_UNSIGNED_BYTE) {
                scalable_internal_format = GL_R8;
            }else if(gltype == GL_UNSIGNED_SHORT){
                scalable_internal_format = GL_R16;
            }else if(gltype == GL_UNSIGNED_INT){
                scalable_internal_format = GL_R32UI;
            }else if(gltype == GL_FLOAT){
                scalable_internal_format = GL_R32F;
            }else{
                scalable_internal_format = GL_RED;
            }
        }else if(glformat == GL_RG) {
            if(gltype == GL_UNSIGNED_BYTE) {
                scalable_internal_format = GL_RG8;
            }else if(gltype == GL_UNSIGNED_SHORT){
                scalable_internal_format = GL_RG16;
            }else if(gltype == GL_UNSIGNED_INT){
                scalable_internal_format = GL_RG32UI;
            }else if(gltype == GL_FLOAT){
                scalable_internal_format = GL_RG32F;
            }else{
                scalable_internal_format = GL_RG;
            }
        }else if(glformat == GL_RGB || glformat == GL_BGR) {
            if(gltype == GL_UNSIGNED_BYTE) {
                scalable_internal_format = GL_RGB8;
            }else if(gltype == GL_UNSIGNED_SHORT){
                scalable_internal_format = GL_RGB16;
            }else if(gltype == GL_UNSIGNED_INT){
                scalable_internal_format = GL_RGB32UI;
            }else if(gltype == GL_FLOAT){
                scalable_internal_format = GL_RGB32F;
            }else{
                scalable_internal_format = GL_RGB;
            }
        }else if(glformat == GL_RGBA || glformat == GL_BGRA) {
            if(gltype == GL_UNSIGNED_BYTE) {
                scalable_internal_format = GL_RGBA8;
            }else if(gltype == GL_UNSIGNED_SHORT){
                scalable_internal_format = GL_RGBA16;
            }else if(gltype == GL_UNSIGNED_INT){
                scalable_internal_format = GL_RGBA32UI;
            }else if(gltype == GL_FLOAT){
                scalable_internal_format = GL_RGBA32F;
            }else{
                scalable_internal_format = GL_RGBA;
            }
        }
    }

    template<typename T>
    static GlPixFormat FromType()
    {
        GlPixFormat fmt;
        fmt.glformat = GlFormatTraits<T>::glformat;
        fmt.gltype = GlFormatTraits<T>::gltype;
        fmt.scalable_internal_format = GlFormatTraits<T>::glinternalformat;
        return fmt;
    }

    GLint glformat;
    GLenum gltype;
    GLint scalable_internal_format;
};

}
