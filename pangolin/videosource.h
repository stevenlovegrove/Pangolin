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

#ifndef PANGOLIN_VIDEOSOURCE_H
#define PANGOLIN_VIDEOSOURCE_H

#include "pangolin.h"
#ifdef HAVE_DC1394

#include <dc1394/dc1394.h>

#ifndef _WIN32
#include <unistd.h>
#endif

namespace pangolin
{

struct VideoException : std::exception
{
  VideoException(std::string str) : desc(str) {}
  ~VideoException() throw() {}
  const char* what() const throw() { return desc.c_str(); }
  std::string desc;
};

class FirewireFrame
{
  friend class FirewireVideo;
public:
  bool isValid() { return frame; }
  unsigned char* Image() { return frame ? frame->image : 0; }
  unsigned Width() const { return frame ? frame->size[0] : 0; }
  unsigned Height() const { return frame ? frame->size[1] : 0; }

protected:
  FirewireFrame(dc1394video_frame_t* frame) : frame(frame) {}
  dc1394video_frame_t *frame;
};

struct Guid
{
    Guid(uint64_t guid):guid(guid){}
    uint64_t guid;
};

class FirewireVideo
{
public:
  FirewireVideo(
    unsigned deviceid = 0,
    dc1394video_mode_t video_mode = DC1394_VIDEO_MODE_640x480_RGB8,
    dc1394framerate_t framerate = DC1394_FRAMERATE_30,
    dc1394speed_t iso_speed = DC1394_ISO_SPEED_400,
    int dma_buffers = 10
  );

  FirewireVideo(
    Guid guid,
    dc1394video_mode_t video_mode = DC1394_VIDEO_MODE_640x480_RGB8,
    dc1394framerate_t framerate = DC1394_FRAMERATE_30,
    dc1394speed_t iso_speed = DC1394_ISO_SPEED_400,
    int dma_buffers = 10
  );

  ~FirewireVideo();

  int Width() const { return width; }
  int Height() const { return height; }

  void Start();
  void Stop();

  //! Copy the next frame from the camera to image.
  //! Optionally wait for a frame if one isn't ready
  //! Returns true iff image was copied
  bool GrabNext( unsigned char* image, bool wait = true );

  //! Copy the newest frame from the camera to image
  //! discarding all older frames.
  //! Optionally wait for a frame if one isn't ready
  //! Returns true iff image was copied
  bool GrabNewest( unsigned char* image, bool wait = true );

  //! Return object containing reference to image data within
  //! DMA buffer. The FirewireFrame must be returned to
  //! signal that it can be reused with a corresponding PutFrame()
  FirewireFrame GetNext(bool wait = true);

  //! Return object containing reference to newest image data within
  //! DMA buffer discarding old images. The FirewireFrame must be
  //! returned to signal that it can be reused with a corresponding PutFrame()
  FirewireFrame GetNewest(bool wait = true);

  //! Return FirewireFrame object. Data held by FirewireFrame is
  //! invalidated on return.
  void PutFrame(FirewireFrame& frame);

  float GetShutterTime() const;

  float GetGamma() const;

protected:
  void init_camera(
    uint64_t guid, int dma_frames,
    dc1394speed_t iso_speed,
    dc1394video_mode_t video_mode,
    dc1394framerate_t framerate
  );

  bool running;
  dc1394camera_t *camera;
  unsigned int width, height;
  //dc1394featureset_t features;
  dc1394_t * d;
  dc1394camera_list_t * list;
  mutable dc1394error_t err;
};

}


#endif // HAVE_DC1394
#endif // PANGOLIN_VIDEOSOURCE_H
