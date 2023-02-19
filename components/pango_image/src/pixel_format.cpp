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

#include <pangolin/image/pixel_format.h>

#include <stdexcept>

namespace pangolin
{

const std::unordered_map<std::string, PixelFormat>& KnownPixelTypes()
{
  using namespace sophus;
  static std::unordered_map<std::string, PixelFormat> table = {
      {"GRAY8", {NumberType::fixed_point, 1, 1}},
      {"GRAY10", {NumberType::fixed_point, 1, 1}},
      {"GRAY12", {NumberType::fixed_point, 1, 1}},
      {"GRAY16LE", {NumberType::fixed_point, 1, 1}},
      {"GRAY32", {NumberType::fixed_point, 1, 1}},
      // {"Y400A",    {NumberType::fixed_point, ?, ?}},
      {"RGB24", {NumberType::fixed_point, 3, 1}},
      {"BGR24", {NumberType::fixed_point, 3, 1}},
      {"RGB48", {NumberType::fixed_point, 3, 2}},
      {"BGR48", {NumberType::fixed_point, 3, 2}},
      {"YUYV422", {NumberType::fixed_point, 4, 2}},
      {"UYVY422", {NumberType::fixed_point, 4, 2}},
      {"RGBA32", {NumberType::fixed_point, 4, 1}},
      {"BGRA32", {NumberType::fixed_point, 4, 1}},
      {"RGBA64", {NumberType::fixed_point, 4, 2}},
      {"BGRA64", {NumberType::fixed_point, 4, 2}},
      {"GRAY32F", {NumberType::floating_point, 1, 4}},
      {"GRAY64F", {NumberType::floating_point, 1, 8}},
      {"RGB48F", {NumberType::floating_point, 3, 2}},
      {"BGR48F", {NumberType::floating_point, 3, 2}},
      {"RGBA64F", {NumberType::floating_point, 4, 2}},
      {"BGRA64F", {NumberType::floating_point, 4, 2}},
      {"RGB96F", {NumberType::floating_point, 3, 4}},
      {"BGR96F", {NumberType::floating_point, 3, 4}},
      {"RGBA128F", {NumberType::floating_point, 4, 4}},
      {"ABGR128F", {NumberType::floating_point, 4, 4}},
  };

  return table;
}

PixelFormat PixelFormatFromString(const std::string& format)
{
  const auto& m = KnownPixelTypes();
  auto it = m.find(format);
  if (it != m.end()) return it->second;
  throw std::runtime_error(std::string("Unknown Format: ") + format);
}

std::string ToString(const PixelFormat& fmt)
{
  for (auto& [key, val] : KnownPixelTypes()) {
    if (val == fmt) return key;
  }
  throw std::runtime_error(std::string("Unknown Format"));
}

}  // namespace pangolin
