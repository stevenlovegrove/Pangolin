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

#ifndef PANGOLIN_FIREWIRE_H
#define PANGOLIN_FIREWIRE_H

#include <pangolin/pangolin.h>
#include <pangolin/video.h>

#include <dc1394/dc1394.h>

#ifndef _WIN32
#include <unistd.h>
#endif



namespace pangolin
{

std::string Dc1394ColorCodingToString(dc1394color_coding_t coding);

dc1394color_coding_t Dc1394ColorCodingFromString(std::string coding);

void Dc1394ModeDetails(dc1394video_mode_t mode, unsigned& w, unsigned& h, std::string& format );

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

class FirewireVideo : public VideoInterface
{
public:
  const static int MAX_FR = -1;
  const static int EXT_TRIG = -1;

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

  FirewireVideo(
      Guid guid,
      dc1394video_mode_t video_mode,
      int framerate,
      uint32_t width, uint32_t height,
      uint32_t left, uint32_t top,
      dc1394speed_t iso_speed,
      int dma_buffers, bool reset_at_boot=false
  );

  FirewireVideo(
      unsigned deviceid,
      dc1394video_mode_t video_mode,
      int framerate,
      uint32_t width, uint32_t height,
      uint32_t left, uint32_t top,
      dc1394speed_t iso_speed,
      int dma_buffers, bool reset_at_boot=false
  );

  ~FirewireVideo();

  //! Implement VideoSource::Width()
  unsigned Width() const { return width; }

  //! Implement VideoSource::Height()
  unsigned Height() const { return height; }

  //! Implement VideoSource::SizeBytes()
  size_t SizeBytes() const;

  //! Implement VideoSource::PixFormat()
  std::string PixFormat() const;

  //! Implement VideoSource::Start()
  void Start();

  //! Implement VideoSource::Stop()
  void Stop();

  //! Implement VideoSource::GrabNext()
  bool GrabNext( unsigned char* image, bool wait = true );

  //! Implement VideoSource::GrabNewest()
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

  //! return absolute shutter value
  float GetShutterTime() const;

  //! set absolute shutter value
  void SetShutterTime(float val);

  //! set auto shutter value
  void SetAutoShutterTime();

  //! return absolute gain value
  float GetGain() const;

  //! set absolute shutter value
  void SetGain(float val);

  //! set auto shutter value
  void SetAutoGain();

  //! return absolute gamma value
  float GetGamma() const;

  //! return quantised shutter value
  void SetShutterTimeQuant(int shutter);

  //! set the trigger to internal, i.e. determined by video mode
  void SetInternalTrigger();

  //! set the trigger to external
  void SetExternalTrigger(
      dc1394trigger_mode_t mode=DC1394_TRIGGER_MODE_0,
      dc1394trigger_polarity_t polarity=DC1394_TRIGGER_ACTIVE_HIGH,
      dc1394trigger_source_t source=DC1394_TRIGGER_SOURCE_0
  );

protected:

  void init_camera(
    uint64_t guid, int dma_frames,
    dc1394speed_t iso_speed,
    dc1394video_mode_t video_mode,
    dc1394framerate_t framerate
  );

  void init_format7_camera(
      uint64_t guid, int dma_frames,
      dc1394speed_t iso_speed,
      dc1394video_mode_t video_mode,
      int framerate,
      uint32_t width, uint32_t height,
      uint32_t left, uint32_t top, bool reset_at_boot
  );

  static int nearest_value(int value, int step, int min, int max);
  static double bus_period_from_iso_speed(dc1394speed_t iso_speed);

  bool running;
  dc1394camera_t *camera;
  unsigned width, height, top, left;
  //dc1394featureset_t features;
  dc1394_t * d;
  dc1394camera_list_t * list;
  mutable dc1394error_t err;

};

}

#endif // PANGOLIN_FIREWIRE_H
