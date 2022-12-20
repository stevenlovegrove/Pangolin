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
#include <pangolin/utils/httplib.h>
#include <pangolin/utils/memstreambuf.h>
#include <pangolin/utils/string.h>

#include <fstream>

namespace pangolin
{

// PNG
IntensityImage<> LoadPng(std::istream& in);
void SavePng(
    const IntensityImage<>& image, std::ostream& out, bool top_line_first,
    int zlib_compression_level);

// JPG
IntensityImage<> LoadJpg(std::istream& in);
void SaveJpg(const IntensityImage<>& image, std::ostream& out, float quality);

// PPM
IntensityImage<> LoadPpm(std::istream& in);
void SavePpm(
    const IntensityImage<>& image, std::ostream& out, bool top_line_first);

// TGA
IntensityImage<> LoadTga(std::istream& in);

// Pango
IntensityImage<> LoadPango(const std::string& filename);
void SavePango(
    const IntensityImage<>& image, const std::string& filename,
    bool top_line_first);

// EXR
IntensityImage<> LoadExr(std::istream& source);
void SaveExr(
    const IntensityImage<>& image, const std::string& filename,
    bool top_line_first);

// BMP
IntensityImage<> LoadBmp(std::istream& source);
void SaveBmp(
    const IntensityImage<>& image, std::ostream& out, bool top_line_first);

// ZSTD (https://github.com/facebook/zstd)
IntensityImage<> LoadZstd(std::istream& in);
void SaveZstd(
    const IntensityImage<>& image, std::ostream& out, int compression_level);

// https://github.com/lz4/lz4
IntensityImage<> LoadLz4(std::istream& in);
void SaveLz4(
    const IntensityImage<>& image, std::ostream& out, int compression_level);

// packed 12 bit image (obtained from unpacked 16bit)
IntensityImage<> LoadPacked12bit(std::istream& in);
void SavePacked12bit(const IntensityImage<>& image, std::ostream& out);

// LibRaw raw camera files
IntensityImage<> LoadLibRaw(const std::string& filename);

// TIFF
IntensityImage<> LoadTiff(const std::string& filename);

IntensityImage<> LoadImage(std::istream& in, ImageFileType file_type)
{
  switch (file_type) {
    case ImageFileTypePng:
      return LoadPng(in);
    case ImageFileTypeJpg:
      return LoadJpg(in);
    case ImageFileTypePpm:
      return LoadPpm(in);
    case ImageFileTypeTga:
      return LoadTga(in);
    case ImageFileTypeZstd:
      return LoadZstd(in);
    case ImageFileTypeLz4:
      return LoadLz4(in);
    case ImageFileTypeP12b:
      return LoadPacked12bit(in);
    case ImageFileTypeExr:
      return LoadExr(in);
    case ImageFileTypeBmp:
      return LoadBmp(in);
    default:
      throw std::runtime_error(
          "Unable to load image file-type through std::istream");
  }
}

IntensityImage<> LoadImage(const std::string& filename, ImageFileType file_type)
{
  switch (file_type) {
    case ImageFileTypePng:
    case ImageFileTypeJpg:
    case ImageFileTypePpm:
    case ImageFileTypeTga:
    case ImageFileTypeZstd:
    case ImageFileTypeLz4:
    case ImageFileTypeP12b:
    case ImageFileTypeExr:
    case ImageFileTypeBmp: {
      std::ifstream ifs(filename, std::ios_base::in | std::ios_base::binary);
      PANGO_THROW_IF(!ifs.good());
      return LoadImage(ifs, file_type);
    }
    case ImageFileTypePango:
      return LoadPango(filename);
    case ImageFileTypeArw:
      return LoadLibRaw(filename);
    case ImageFileTypeTiff:
      return LoadTiff(filename);
    default:
      throw std::runtime_error(
          "Unsupported image file type, '" + filename + "'");
  }
}

struct Uri {
  std::string server;
  std::string path;
};

Uri splitUri(const std::string& uri)
{
  const size_t scheme_end = uri.find("://");
  PANGO_THROW_IF(scheme_end == uri.npos);

  const size_t first_slash = uri.find('/', scheme_end + 3);
  if (first_slash == uri.npos) {
    return {uri, "/"};
  } else {
    return {uri.substr(0, first_slash), uri.substr(first_slash)};
  }
}

IntensityImage<> LoadHttpImage(const std::string& uri)
{
  auto split = splitUri(uri);

  httplib::Client cli(split.server);
  auto res = cli.Get(split.path);

  if (res->status == 200) {
    const ImageFileType filetype =
        FileTypeMagic((uint8_t*)res->body.data(), res->body.size());
    IStreamMemoryWrapper s((uint8_t*)res->body.data(), res->body.size());
    auto img = LoadImage(s, filetype);
    return img;
  } else {
    PANGO_WARN(
        "GET Request failed. {}. {} bytes in body",
        httplib::detail::status_message(res->status), res->body.size());
    return IntensityImage<>();
  }
}

IntensityImage<> LoadImage(const std::string& filename)
{
  if (startsWith(filename, "http")) {
    return LoadHttpImage(filename);
  } else {
    ImageFileType file_type = FileType(filename);
    return LoadImage(filename, file_type);
  }
}

void SaveImage(
    const IntensityImage<>& image, std::ostream& out, ImageFileType file_type,
    bool top_line_first, float quality)
{
  switch (file_type) {
    case ImageFileTypePng:
      // map quality [0..100] to PNG compression levels [0..9]
      return SavePng(image, out, top_line_first, int(quality * 0.09));
    case ImageFileTypeJpg:
      return SaveJpg(image, out, quality);
    case ImageFileTypePpm:
      return SavePpm(image, out, top_line_first);
    case ImageFileTypeZstd:
      return SaveZstd(image, out, (int)quality);
    case ImageFileTypeLz4:
      return SaveLz4(image, out, (int)quality);
    case ImageFileTypeP12b:
      return SavePacked12bit(image, out);
    case ImageFileTypeBmp:
      return SaveBmp(image, out, top_line_first);
    default:
      throw std::runtime_error(
          "Unable to save image file-type through std::istream");
  }
}

void SaveImage(
    const IntensityImage<>& image, const std::string& filename,
    ImageFileType file_type, bool top_line_first, float quality)
{
  switch (file_type) {
    case ImageFileTypePng:
    case ImageFileTypeJpg:
    case ImageFileTypePpm:
    case ImageFileTypeZstd:
    case ImageFileTypeLz4:
    case ImageFileTypeP12b:
    case ImageFileTypeBmp: {
      std::ofstream ofs(filename, std::ios_base::binary);
      return SaveImage(image, ofs, file_type, top_line_first, quality);
    }
    case ImageFileTypeExr:
      return SaveExr(image, filename, top_line_first);
    case ImageFileTypePango:
      return SavePango(image, filename, top_line_first);
    default:
      throw std::runtime_error(
          "Unsupported image file type, '" + filename + "'");
  }
}

void SaveImage(
    const IntensityImage<>& image, const std::string& filename,
    bool top_line_first, float quality)
{
  const std::string ext = FileLowercaseExtention(filename);
  const ImageFileType file_type = FileTypeExtension(ext);
  SaveImage(image, filename, file_type, top_line_first, quality);
}

}  // namespace pangolin
