/* This file is part of the Pangolin Project.
 * http://github.com/stevenlovegrove/Pangolin
 *
 * Copyright (c) 2011-2013 Steven Lovegrove
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

// Pangolin video output supports various formats using
// different 3rd party libraries. (Only one right now)
//
// VideoOutput URI's take the following form:
//  scheme:[param1=value1,param2=value2,...]//device
//
// scheme = ffmpeg
//
// ffmpeg - encode to compressed file using ffmpeg
//  fps : fps to embed in encoded file.
//  bps : bits per second
//  unique_filename : append unique suffix if file already exists
//
//  e.g. ffmpeg://output_file.avi
//  e.g. ffmpeg:[fps=30,bps=1000000,unique_filename]//output_file.avi

#include <pangolin/utils/uri.h>
#include <pangolin/video/video_output_interface.h>

#include <memory>

namespace pangolin
{

//! VideoOutput wrap to generically construct instances of VideoOutputInterface.
class PANGOLIN_EXPORT VideoOutput : public VideoOutputInterface
{
  public:
  VideoOutput();
  VideoOutput(VideoOutput&& other) = default;
  VideoOutput(std::string const& uri);
  ~VideoOutput();

  bool IsOpen() const;
  void Open(std::string const& uri);
  void Close();

  std::vector<StreamInfo> const& Streams() const override;

  void SetStreams(
      std::vector<StreamInfo> const& streams, std::string const& uri = "",
      picojson::value const& properties = picojson::value()) override;

  int WriteStreams(
      uint8_t const* data,
      picojson::value const& frame_properties = picojson::value()) override;

  bool IsPipe() const override;

  void AddStream(RuntimePixelType const& pf, sophus::ImageShape shape);

  void AddStream(RuntimePixelType const& pf, sophus::ImageSize size);

  void SetStreams(
      std::string const& uri = "",
      picojson::value const& properties = picojson::value());

  size_t SizeBytes(void) const;

  std::vector<sophus::ImageView<uint8_t>> GetOutputImages(
      uint8_t* buffer) const;

  std::vector<sophus::ImageView<uint8_t>> GetOutputImages(
      std::vector<uint8_t>& buffer) const;

  protected:
  std::vector<StreamInfo> streams;
  Uri uri;
  std::unique_ptr<VideoOutputInterface> recorder;
};

}  // namespace pangolin
