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

#include <pangolin/image/image_io.h>

#include <fstream>

namespace pangolin {

TypedImage LoadTga(const std::string& filename);
TypedImage LoadPng(const std::string& filename);
TypedImage LoadJpg(const std::string& filename);
TypedImage LoadPpm(const std::string& filename);
TypedImage LoadPango(const std::string& filename);

void SavePng(const Image<unsigned char>& image, const pangolin::PixelFormat& fmt, const std::string& filename, bool top_line_first);
void SavePpm(const Image<unsigned char>& image, const pangolin::PixelFormat& fmt, const std::string& filename, bool top_line_first);
void SaveExr(const Image<unsigned char>& image, const pangolin::PixelFormat& fmt, const std::string& filename, bool top_line_first);
void SavePango(const Image<unsigned char>& image, const pangolin::PixelFormat& fmt, const std::string& filename, bool top_line_first);

TypedImage LoadImage(const std::string& filename, ImageFileType file_type)
{
    switch (file_type) {
    case ImageFileTypeTga:
        return LoadTga(filename);
    case ImageFileTypePng:
        return LoadPng(filename);
    case ImageFileTypeJpg:
        return LoadJpg(filename);
    case ImageFileTypePpm:
        return LoadPpm(filename);
    case ImageFileTypePango:
        return LoadPango(filename);
    default:
        throw std::runtime_error("Unsupported image file type, '" + filename + "'");
    }
}

TypedImage LoadImage(const std::string& filename)
{
    ImageFileType file_type = FileType(filename);
    return LoadImage( filename, file_type );
}

TypedImage LoadImage(
    const std::string& filename,
    const PixelFormat& raw_fmt,
    size_t raw_width, size_t raw_height, size_t raw_pitch
) {
    TypedImage img(raw_width, raw_height, raw_fmt, raw_pitch);

    // Read from file, row at a time.
    std::ifstream bFile( filename.c_str(), std::ios::in | std::ios::binary );
    for(size_t r=0; r<img.h; ++r) {
        bFile.read( (char*)img.ptr + r*img.pitch, img.pitch );
        if(bFile.fail()) {
            pango_print_warn("Unable to read raw image file to completion.");
            break;
        }
    }
    return img;
}

void SaveImage(const Image<unsigned char>& image, const pangolin::PixelFormat& fmt, const std::string& filename, ImageFileType file_type, bool top_line_first)
{
    switch (file_type) {
    case ImageFileTypePng:
        return SavePng(image, fmt, filename, top_line_first);
    case ImageFileTypePpm:
        return SavePpm(image, fmt, filename, top_line_first);
    case ImageFileTypeExr:
        return SaveExr(image, fmt, filename, top_line_first);
    case ImageFileTypePango:
        return SavePango(image, fmt, filename, top_line_first);
    default:
        throw std::runtime_error("Unsupported image file type, '" + filename + "'");
    }
}

void SaveImage(const Image<unsigned char>& image, const pangolin::PixelFormat& fmt, const std::string& filename, bool top_line_first)
{
    const std::string ext = FileLowercaseExtention(filename);
    const ImageFileType file_type = FileTypeExtension(ext);
    SaveImage(image, fmt, filename,file_type, top_line_first);
}

void SaveImage(const TypedImage& image, const std::string& filename, bool top_line_first)
{
    SaveImage(image, image.fmt, filename, top_line_first);
}

}
