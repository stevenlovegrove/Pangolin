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

#include <pangolin/video/drivers/debayer.h>

#ifdef HAVE_DC1394
#include <dc1394/conversions.h>
#endif

namespace pangolin
{

DebayerVideo::DebayerVideo(VideoInterface* src, color_filter_t tile, bayer_method_t method)
    : size_bytes(0), buffer(0), tile(tile), method(method)
{
    if(!src) {
        throw VideoException("DebayerVideo: VideoInterface in must not be null");
    }
    videoin.push_back(src);

#ifndef HAVE_DC1394
    pango_print_warn("debayer: dc1394 unavailable for debayering. Using simple downsampling method instead.\n");
    this->method = BAYER_METHOD_DOWNSAMPLE;
#endif

    const pangolin::VideoPixelFormat rgb_format = pangolin::VideoFormatFromString("RGB24");
    for(size_t s=0; s< src->Streams().size(); ++s) {
        size_t w = src->Streams()[s].Width();
        size_t h = src->Streams()[s].Height();
        if(this->method==BAYER_METHOD_DOWNSAMPLE) {
            w = w/2;
            h = h/2;
        }
        streams.push_back(pangolin::StreamInfo( rgb_format, w, h, w*rgb_format.bpp / 8, (unsigned char*)0 + size_bytes ));
        size_bytes += w*h*rgb_format.bpp / 8;
    }

    buffer = new unsigned char[src->SizeBytes()];
}

DebayerVideo::~DebayerVideo()
{
    delete[] buffer;
    delete videoin[0];
}

//! Implement VideoInput::Start()
void DebayerVideo::Start()
{
    videoin[0]->Start();
}

//! Implement VideoInput::Stop()
void DebayerVideo::Stop()
{
    videoin[0]->Stop();
}

//! Implement VideoInput::SizeBytes()
size_t DebayerVideo::SizeBytes() const
{
    return size_bytes;
}

//! Implement VideoInput::Streams()
const std::vector<StreamInfo>& DebayerVideo::Streams() const
{
    return streams;
}

void DownsampleDebayer(Image<unsigned char>& out, const Image<unsigned char>& in, color_filter_t tile)
{
    switch(tile) {
      case DC1394_COLOR_FILTER_RGGB:
        for(size_t y=0; y<out.h; ++y) {
          for(size_t x=0; x<out.w; ++x) {
             out.ptr[3*(y*out.w+x)+2] = in.ptr[(2*y+1)*in.pitch + 2*x + 1];
             out.ptr[3*(y*out.w+x)+1] = (in.ptr[2*y*in.pitch + 2*x + 1] + in.ptr[(2*y+1)*in.pitch + 2*x]) >> 1;
             out.ptr[3*(y*out.w+x)+0] = in.ptr[2*y*in.pitch + 2*x];
          }
        }
        break;
      case DC1394_COLOR_FILTER_GBRG:
        for(size_t y=0; y<out.h; ++y) {
          for(size_t x=0; x<out.w; ++x) {
             out.ptr[3*(y*out.w+x)+2] = in.ptr[2*y*in.pitch + 2*x + 1];
             out.ptr[3*(y*out.w+x)+1] = (in.ptr[2*y*in.pitch + 2*x] + in.ptr[(2*y+1)*in.pitch + 2*x + 1]) >> 1;
             out.ptr[3*(y*out.w+x)+0] = in.ptr[(2*y+1)*in.pitch + 2*x];

          }
        }
        break;
      case DC1394_COLOR_FILTER_GRBG:
        for(size_t y=0; y<out.h; ++y) {
          for(size_t x=0; x<out.w; ++x) {
             out.ptr[3*(y*out.w+x)+2] = in.ptr[(2*y+1)*in.pitch + 2*x];
             out.ptr[3*(y*out.w+x)+1] = (in.ptr[2*y*in.pitch + 2*x] + in.ptr[(2*y+1)*in.pitch + 2*x + 1]) >> 1;
             out.ptr[3*(y*out.w+x)+0] = in.ptr[2*y*in.pitch + 2*x + 1];
          }
        }
        break;
      case DC1394_COLOR_FILTER_BGGR:
        for(size_t y=0; y<out.h; ++y) {
          for(size_t x=0; x<out.w; ++x) {
             out.ptr[3*(y*out.w+x)+2] = in.ptr[2*y*in.pitch + 2*x];
             out.ptr[3*(y*out.w+x)+1] = (in.ptr[2*y*in.pitch + 2*x + 1] + in.ptr[(2*y+1)*in.pitch + 2*x]) >> 1;
             out.ptr[3*(y*out.w+x)+0] = in.ptr[(2*y+1)*in.pitch + 2*x + 1];
          }
        }
        break;
    }
}

//! Implement VideoInput::GrabNext()
bool DebayerVideo::GrabNext( unsigned char* image, bool wait )
{    
    if(videoin[0]->GrabNext(buffer,wait)) {
        for(size_t s=0; s<streams.size(); ++s) {
            Image<unsigned char> img_in  = videoin[0]->Streams()[s].StreamImage(buffer);
            Image<unsigned char> img_out = Streams()[s].StreamImage(image);

#ifdef HAVE_DC1394
            dc1394_bayer_decoding_8bit(
                img_in.ptr, img_out.ptr, img_in.w, img_in.h,
                (dc1394color_filter_t)tile, (dc1394bayer_method_t)method
            );
#else
            // use our simple debayering instead
            DownsampleDebayer(img_out, img_in, tile);
#endif
        }
        return true;
    }else{
        return false;
    }
}

//! Implement VideoInput::GrabNewest()
bool DebayerVideo::GrabNewest( unsigned char* image, bool wait )
{
    return GrabNext(image,wait);
}

std::vector<VideoInterface*>& DebayerVideo::InputStreams()
{
    return videoin;
}

color_filter_t DebayerVideo::ColorFilterFromString(std::string str)
{
  if(!str.compare("rggb") || !str.compare("RGGB")) return DC1394_COLOR_FILTER_RGGB;
  else if(!str.compare("gbrg") || !str.compare("GBRG")) return DC1394_COLOR_FILTER_GBRG;
  else if(!str.compare("grbg") || !str.compare("GRBG")) return DC1394_COLOR_FILTER_GRBG;
  else if(!str.compare("bggr") || !str.compare("BGGR")) return DC1394_COLOR_FILTER_BGGR;
  else {
     pango_print_error("Debayer error, %s is not a valid tile tile using RGGB\n", str.c_str());
     return DC1394_COLOR_FILTER_RGGB;
  }
}

}
