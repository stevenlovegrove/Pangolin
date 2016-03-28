/* This file is part of the Pangolin Project.
 * http://github.com/stevenlovegrove/Pangolin
 *
 * Copyright (c) 2015 Steven Lovegrove
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

#ifndef PANGOLIN_VIDEO_PLEORA_H
#define PANGOLIN_VIDEO_PLEORA_H

#include <pangolin/pangolin.h>
#include <pangolin/video/video.h>

#include <PvDevice.h>
#include <PvDeviceGEV.h>
#include <PvDeviceU3V.h>
#include <PvStream.h>
#include <PvStreamGEV.h>
#include <PvStreamU3V.h>
#include <PvBuffer.h>

#include <PvSystem.h>

#include <stdlib.h>
#include <list>
#include <thread>
#include <mutex>

namespace pangolin
{

struct GrabbedBuffer {

  inline GrabbedBuffer(PvBuffer* b,PvResult r,bool v)
      : buff(b), res(r), valid(v)
  {
  }

  PvBuffer* buff;
  PvResult res;
  bool valid;

};

typedef std::list<GrabbedBuffer> GrabbedBufferList;

typedef std::list<PvBuffer *> BufferList;

class PANGOLIN_EXPORT PleoraVideo :
        public VideoInterface, public VideoPropertiesInterface, public BufferAwareVideoInterface
{
public:

    static const size_t DEFAULT_BUFFER_COUNT = 30;

    PleoraVideo(const char *model_name, const char *serial_num, size_t index, size_t bpp = 8, size_t binX = 1, size_t binY = 1, size_t buffer_count = DEFAULT_BUFFER_COUNT,
                size_t desired_size_x = 0, size_t desired_size_y = 0, size_t desired_pos_x = 0, size_t desired_pos_y = 0, int again = -1, double exposure = 0,
                bool ext_trig=false, size_t analog_black_level=0, bool use_separate_thread = false);
    ~PleoraVideo();

    void Start();

    void Stop();

    size_t SizeBytes() const;

    const std::vector<StreamInfo>& Streams() const;

    bool GrabNext( unsigned char* image, bool wait = true );

    bool GrabNewest( unsigned char* image, bool wait = true );

    void SetGain(int64_t val);

    int64_t GetGain();

    void SetAnalogBlackLevel(int64_t val);

    int64_t GetAnalogBlackLevel();

    void SetExposure(double val);

    double GetExposure();

    void SetupTrigger(bool triggerActive, int64_t triggerSource, int64_t acquisitionMode);

    const json::value& DeviceProperties() const {
        return device_properties;
    }

    const json::value& FrameProperties() const {
        return frame_properties;
    }

    const uint32_t AvailableFrames();

    const bool DropNFrames(uint32_t n);

protected:
    void InitDevice(const char *model_name, const char *serial_num, size_t index);
    void DeinitDevice();

    void SetDeviceParams(
        size_t bpp,  size_t binX, size_t binY,
        size_t desired_size_x, size_t desired_size_y, size_t desired_pos_x, size_t desired_pos_y,
        int analog_gain, double exposure, bool ext_trig, size_t analog_black_level
    );

    void InitStream();
    void DeinitStream();

    void InitPangoStreams();

    void InitBuffers(size_t buffer_count);

    void DeinitBuffers();

    template<typename T>
    T DeviceParam(const char* name);

    template<typename T>
    bool SetDeviceParam(const char* name, T val);

    template<typename T>
    T StreamParam(const char* name);

    template<typename T>
    bool SetStreamParam(const char* name, T val);

    bool ParseBuffer(PvBuffer* lBuffer,  unsigned char* image);

    std::vector<StreamInfo> streams;
    json::value device_properties;
    json::value frame_properties;

    size_t size_bytes;

    // Pleora handles
    PvSystem* lPvSystem;
    const PvDeviceInfo* lDeviceInfo;
    PvDevice* lDevice;
    PvStream* lStream;

    // Genicam device parameters
    PvGenParameterArray* lDeviceParams;
    PvGenCommand* lStart;
    PvGenCommand* lStop;

    PvGenInteger* lAnalogGain;
    PvGenInteger* lAnalogBlackLevel;
    PvGenFloat*   lExposure;
    PvGenEnum*    lAquisitionMode;
    PvGenEnum*    lTriggerSource;
    PvGenEnum*    lTriggerMode;
    PvGenFloat*   lTemperatureCelcius;

    // Genicam stream parameters
    PvGenParameterArray* lStreamParams;

    BufferList lBufferList;
    bool stand_alone_grab_thread;
    std::thread grab_thread;
    bool quit_grab_thread;
    GrabbedBufferList lGrabbedBuffList;
    std::mutex lStreamMtx;
    std::mutex grabbedBuffListMtx;
};

}

#endif // PANGOLIN_VIDEO_PLEORA_H
