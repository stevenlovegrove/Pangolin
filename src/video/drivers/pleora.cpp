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

#include <pangolin/video/drivers/pleora.h>
#include <unistd.h>
#include <pangolin/utils/timer.h>

namespace pangolin
{

inline void ThrowOnFailure(const PvResult& res)
{
    if(res.IsFailure()) {
        throw std::runtime_error("Failure: " + std::string(res.GetCodeString().GetAscii()) );
    }
}

template<typename T>
struct PleoraParamTraits;

template<> struct PleoraParamTraits<bool> {
    typedef PvGenBoolean PvType;
};
template<> struct PleoraParamTraits<int64_t> {
    typedef PvGenInteger PvType;
};
template<> struct PleoraParamTraits<float> {
    typedef PvGenFloat PvType;
};
template<> struct PleoraParamTraits<std::string> {
    typedef PvGenString PvType;
};

template<typename T>
T GetParam(PvGenParameterArray* params, const char* name)
{
    typedef typename PleoraParamTraits<T>::PvType PvType;
    PvType* param = dynamic_cast<PvType*>( params->Get(name) );
    if(!param) {
        throw std::runtime_error("Incorrect type");
    }
    T ret;
    PvResult res = param->GetValue(ret);
    if(res.IsFailure()) {
        throw std::runtime_error("Cannot get value: " + std::string(res.GetCodeString().GetAscii()) );
    }
    return ret;
}

template<typename T>
bool SetParam(PvGenParameterArray* params, const char* name, T val)
{
    typedef typename PleoraParamTraits<T>::PvType PvType;
    PvType* param = dynamic_cast<PvType*>( params->Get(name) );
    if(!param) {
        throw std::runtime_error("Unable to get parameter handle: " + std::string(name) );
    }

    if(!param->IsWritable()) {
        throw std::runtime_error("Cannot set value for " + std::string(name) );
    }

    PvResult res = param->SetValue(val);
    if(res.IsFailure()) {
        throw std::runtime_error("Cannot set value: " + std::string(res.GetCodeString().GetAscii()) );
    }
    return true;
}

inline const PvDeviceInfo* SelectDevice( PvSystem& aSystem, const char* model_name = 0, const char* serial_num = 0, size_t index = 0 )
{
    aSystem.Find();

    // Enumerate all devices, select first that matches criteria
    size_t matches = 0;
    for ( uint32_t i = 0; i < aSystem.GetInterfaceCount(); i++ ) {
        const PvInterface *lInterface = dynamic_cast<const PvInterface *>( aSystem.GetInterface( i ) );
        if ( lInterface ) {
            for ( uint32_t j = 0; j < lInterface->GetDeviceCount(); j++ ) {
                const PvDeviceInfo *lDI = dynamic_cast<const PvDeviceInfo *>( lInterface->GetDeviceInfo( j ) );
                if ( lDI && lDI->IsConfigurationValid() ) {
                    if( model_name && strcmp(lDI->GetModelName().GetAscii(), model_name) )
                        continue;
                    if( serial_num && strcmp(lDI->GetSerialNumber().GetAscii(), serial_num) )
                        continue;
                    if(matches == index) {
                        return lDI;
                    }
                    ++matches;
                }
            }
        }
    }

    return 0;
}

VideoPixelFormat PleoraFormat(const PvGenEnum* pfmt)
{
    std::string spfmt = pfmt->ToString().GetAscii();
    if( !spfmt.compare("Mono8") ) {
        return VideoFormatFromString("GRAY8");
    } else if( !spfmt.compare("Mono10p") ) {
        return VideoFormatFromString("GRAY10");
    } else if( !spfmt.compare("Mono12p") ) {
        return VideoFormatFromString("GRAY12");
    } else {
        throw VideoException("Unknown Pleora pixel format", spfmt);
    }
}


uint32_t get_num_valid_elements(GrabbedBufferList l)
{
  uint32_t cnt = 0;
  for(GrabbedBufferList::iterator lIt = l.begin(); lIt != l.end(); ++lIt) {
      if(lIt->valid) cnt++;
  }
  return cnt;
}

PleoraVideo::PleoraVideo(
        const char* model_name, const char* serial_num, size_t index, size_t bpp,  size_t binX, size_t binY, size_t buffer_count,
        size_t desired_size_x, size_t desired_size_y, size_t desired_pos_x, size_t desired_pos_y, int analog_gain, double exposure,
        bool ext_trig, size_t analog_black_level, bool use_separate_thread
    )
    : size_bytes(0), lPvSystem(0), lDevice(0), lStream(0), lDeviceParams(0), lStart(0), lStop(0), lTemperatureCelcius(0), lStreamParams(0),
      stand_alone_grab_thread(use_separate_thread), quit_grab_thread(true)

{
    if(ext_trig) {
        // Safe-start sequence to prevent TIMEOUT on some cameras with external triggering.
        InitDevice(model_name, serial_num, index);
        SetDeviceParams(bpp, binX, binY, desired_size_x, desired_size_y, desired_pos_x, desired_pos_y, analog_gain, exposure, ext_trig, analog_black_level);
        DeinitDevice();
    }

    InitDevice(model_name, serial_num, index);
    SetDeviceParams(bpp, binX, binY, desired_size_x, desired_size_y, desired_pos_x, desired_pos_y, analog_gain, exposure, ext_trig, analog_black_level);
    InitStream();

    InitPangoStreams();
    InitBuffers(buffer_count);

    Start();
}

PleoraVideo::~PleoraVideo()
{
    Stop();
    DeinitBuffers();
    DeinitStream();
    DeinitDevice();
}

void PleoraVideo::InitDevice(
        const char* model_name, const char* serial_num, size_t index
) {
    lPvSystem = new PvSystem();
    if ( !lPvSystem ) {
        throw pangolin::VideoException("Pleora: Unable to create PvSystem");
    }

    lDeviceInfo = SelectDevice(*lPvSystem, model_name, serial_num, index);
    if ( !lDeviceInfo ) {
        delete lPvSystem;
        throw pangolin::VideoException("Pleora: Unable to select device");
    }

    PvResult lResult;
    lDevice = PvDevice::CreateAndConnect( lDeviceInfo, &lResult );
    if ( !lDevice ) {
        delete lPvSystem;
        throw pangolin::VideoException("Pleora: Unable to connect to device", lResult.GetDescription().GetAscii() );
    }

    lDeviceParams = lDevice->GetParameters();
}

void PleoraVideo::DeinitDevice()
{
    if(lDevice) {
        lDevice->Disconnect();
        PvDevice::Free( lDevice );
        lDevice = 0;
    }

    delete lPvSystem;
    lPvSystem = 0;
}

void PleoraVideo::InitStream()
{
    // Setup Stream
    PvResult lResult;
    lStream = PvStream::CreateAndOpen( lDeviceInfo->GetConnectionID(), &lResult );
    if ( !lStream ) {
        DeinitDevice();
        throw pangolin::VideoException("Pleora: Unable to open stream", lResult.GetDescription().GetAscii() );
    }
    lStreamParams = lStream->GetParameters();
}

void PleoraVideo::DeinitStream()
{
    if(lStream) {
        lStream->Close();
        PvStream::Free( lStream );
        lStream = 0;
    }
}

void PleoraVideo::SetDeviceParams(
    size_t bpp,  size_t binX, size_t binY,
    size_t desired_size_x, size_t desired_size_y, size_t desired_pos_x, size_t desired_pos_y,
    int analog_gain, double exposure, bool ext_trig, size_t analog_black_level
) {
    lStart = dynamic_cast<PvGenCommand*>( lDeviceParams->Get( "AcquisitionStart" ) );
    lStop = dynamic_cast<PvGenCommand*>( lDeviceParams->Get( "AcquisitionStop" ) );

    if( bpp == 8) {
        lDeviceParams->SetEnumValue("PixelFormat", PvString("Mono8") );
    } else if(bpp == 10) {
        lDeviceParams->SetEnumValue("PixelFormat", PvString("Mono10p") );
    } else if(bpp == 12) {
        lDeviceParams->SetEnumValue("PixelFormat", PvString("Mono12p") );
    }


    // Get Handles to properties we'll be using.
    lAnalogGain = lDeviceParams->GetInteger("AnalogGain");
    lAnalogBlackLevel = lDeviceParams->GetInteger("AnalogBlackLevel");
    lExposure = lDeviceParams->GetFloat("ExposureTime");
    lAquisitionMode = lDeviceParams->GetEnum("AcquisitionMode");
    lTriggerSource = lDeviceParams->GetEnum("TriggerSource");
    lTriggerMode = lDeviceParams->GetEnum("TriggerMode");
    lTemperatureCelcius = lDeviceParams->GetFloat("DeviceTemperatureCelsius");

    // Setup device binning
    try {
        PvGenInteger* devbinx = lDeviceParams->GetInteger("BinningHorizontal");
        PvGenInteger* devbiny = lDeviceParams->GetInteger("BinningVertical");
        if( devbinx && devbiny && devbinx->IsWritable() && devbiny->IsWritable()) {
            ThrowOnFailure(devbinx->SetValue(binX));
            ThrowOnFailure(devbiny->SetValue(binY));
        }
    } catch(std::runtime_error e) {
        pango_print_error("Binning: %s\n", e.what());
    }

    // Height and width will fail if not multiples of 8.
    if(desired_size_x || desired_size_y) {
        try {
            SetDeviceParam<int64_t>("Width",  desired_size_x);
            SetDeviceParam<int64_t>("Height", desired_size_y);
        } catch(std::runtime_error e) {
            pango_print_error("SetSize: %s\n", e.what());

            int64_t max, min;
            lDeviceParams->GetIntegerRange("Width", max, min );
            desired_size_x = max;
            lDeviceParams->GetIntegerRange("Height", max, min );
            desired_size_y = max;

            try {
                SetDeviceParam<int64_t>("Width",  desired_size_x);
                SetDeviceParam<int64_t>("Height", desired_size_y);
            } catch(std::runtime_error e) {
                pango_print_error("Set Full frame: %s\n", e.what());
            }
        }
    }

    // Attempt to set offset
    try {
        SetDeviceParam<int64_t>("OffsetX", desired_pos_x);
        SetDeviceParam<int64_t>("OffsetY", desired_pos_y);
    } catch(std::runtime_error e) {
        pango_print_error("Set Offset: %s\n", e.what());
    }

    // Attempt to set AnalogGain, Offset
    try {
        SetGain(analog_gain);
        SetAnalogBlackLevel(analog_black_level);
        SetExposure(exposure);
    } catch(std::runtime_error e) {
        pango_print_error("Set Exposure / Gain: %s\n", e.what());
    }

    // Attempt to set Triggering
    try {
        SetupTrigger(ext_trig, 0, 0);
    } catch(std::runtime_error e) {
        pango_print_error("Set Trigger: %s\n", e.what());
    }
}

void PleoraVideo::InitBuffers(size_t buffer_count)
{
    // Reading payload size from device
    const uint32_t lSize = lDevice->GetPayloadSize();

    // Use buffer_count or the maximum number of buffers, whichever is smaller
    const uint32_t lBufferCount = ( lStream->GetQueuedBufferMaximum() < buffer_count ) ?
        lStream->GetQueuedBufferMaximum() :
        buffer_count;

    // Allocate buffers and queue
    for( uint32_t i = 0; i < lBufferCount; i++ ) {
        PvBuffer *lBuffer = new PvBuffer;
        lBuffer->Alloc( static_cast<uint32_t>( lSize ) );
        lBufferList.push_back( lBuffer );
    }
}

void PleoraVideo::DeinitBuffers()
{
    // Free buffers
    for( BufferList::iterator lIt = lBufferList.begin(); lIt != lBufferList.end(); lIt++ ) {
        delete *lIt;
    }
}

void PleoraVideo::InitPangoStreams()
{
    // Get actual width, height and payload size
    const int w = DeviceParam<int64_t>("Width");
    const int h = DeviceParam<int64_t>("Height");
    const uint32_t lSize = lDevice->GetPayloadSize();

    // Setup pangolin for stream
    PvGenEnum* lpixfmt = dynamic_cast<PvGenEnum*>( lDeviceParams->Get("PixelFormat") );
    const VideoPixelFormat fmt = PleoraFormat(lpixfmt);
    streams.push_back(StreamInfo(fmt, w, h, (w*fmt.bpp)/8));
    size_bytes = lSize;
}

const unsigned int PleoraVideo::AvailableFrames()
{
    if(stand_alone_grab_thread) {
        grabbedBuffListMtx.lock();
        uint32_t ve = get_num_valid_elements(lGrabbedBuffList);
        grabbedBuffListMtx.unlock();
        return ve;
    } else {
        return lStream->GetQueuedBufferCount();
    }
}

const bool PleoraVideo::DropNFrames(uint32_t n)
{
    if(n > AvailableFrames()) return false;

    if(stand_alone_grab_thread) {
        grabbedBuffListMtx.lock();
        GrabbedBufferList::iterator lIt = lGrabbedBuffList.begin();
        while(n > 0) {
            //Mark old buffers as invalid so that they can be reclaimed.
            if(lIt->valid) {
                lIt->valid = false;
                --n;
                std::cout << "DropNFrames: marked 1 frame as invalid" << std::endl;
            }
            ++lIt;
        }
        grabbedBuffListMtx.lock();
    } else {
        PvResult lResult;
        PvBuffer *lBuffer = NULL;
        PvResult lOperationResult;
        while(n > 0) {
            lResult = lStream->RetrieveBuffer( &lBuffer, &lOperationResult, 0 );
            if ( !lResult.IsOK() ) {
                return false;
            } else {
                lStream->QueueBuffer( lBuffer );
            }
        }
    }

    return true;
}

void grab_thread_loop(GrabbedBufferList* p_buff_list, bool* quit_grab_thread, PvStream* lStream, std::mutex* plStream_mtx, std::mutex* p_buff_list_mtx)
{
    PvResult lResult;
    PvBuffer *lBuffer = NULL;
    PvResult lOperationResult;

    while(!*quit_grab_thread) {
        p_buff_list_mtx->lock();
        // housekeeping
        GrabbedBufferList::iterator lIt =  p_buff_list->begin();
        while(lIt != p_buff_list->end()) {
            if(!lIt->valid) {
                plStream_mtx->lock();
                lStream->QueueBuffer(lIt->buff);
                plStream_mtx->unlock();
                lIt = p_buff_list->erase(lIt);
                std::cout<<"-------------------> remove buff" <<std::endl;
            } else {
                ++lIt;
            }
        }
        p_buff_list_mtx->unlock();

        // Retrieve next buffer
        plStream_mtx->lock();
        lResult = lStream->RetrieveBuffer( &lBuffer, &lOperationResult, 5000);
        plStream_mtx->unlock();

        if ( !lResult.IsOK() ) {
            if(lResult && !(lResult.GetCode() == PvResult::Code::TIMEOUT)) {
                pango_print_warn("Pleora error: %s,\n", lResult.GetCodeString().GetAscii());
                usleep(100000);
            }
        } else {
            p_buff_list_mtx->lock();
            p_buff_list->push_back(GrabbedBuffer(lBuffer,lOperationResult,true));
            p_buff_list_mtx->unlock();
        }
        std::this_thread::yield();
    }
    std::cout << "Grab thread stopped" << std::endl;
}

void PleoraVideo::Start()
{
    if(lStream->GetQueuedBufferCount() == 0) {
        // Queue all buffers in the stream
        for( BufferList::iterator lIt = lBufferList.begin(); lIt != lBufferList.end(); lIt++ ) {
            lStream->QueueBuffer( *lIt );
        }
        lDevice->StreamEnable();
        lStart->Execute();

        if(stand_alone_grab_thread) {
            quit_grab_thread = false;
            grab_thread = std::thread(grab_thread_loop,&lGrabbedBuffList, &quit_grab_thread, lStream, &lStreamMtx, &grabbedBuffListMtx);
        }
    } else {
        pango_print_warn("PleoraVideo: Already started.\n");
    }
}

void PleoraVideo::Stop()
{
    // stop grab thread
    if(stand_alone_grab_thread) {
        quit_grab_thread = true;
        grab_thread.join();
    }

    if(lStream->GetQueuedBufferCount() > 0) {
        lStop->Execute();
        lDevice->StreamDisable();

        // Abort all buffers from the stream and dequeue
        lStream->AbortQueuedBuffers();
        while ( lStream->GetQueuedBufferCount() > 0 ) {
            PvBuffer *lBuffer = NULL;
            PvResult lOperationResult;
            lStream->RetrieveBuffer( &lBuffer, &lOperationResult );
        }
    } else {
        pango_print_warn("PleoraVideo: Already stopped.\n");
    }
}

size_t PleoraVideo::SizeBytes() const
{
    return size_bytes;
}

const std::vector<StreamInfo>& PleoraVideo::Streams() const
{
    return streams;
}

bool PleoraVideo::ParseBuffer(PvBuffer* lBuffer,  unsigned char* image)
{
  pangolin::basetime start;
  pangolin::basetime end;
  if ( lBuffer->GetPayloadType() == PvPayloadTypeImage ) {
      start = pangolin::TimeNow();
      PvImage *lImage = lBuffer->GetImage();
      end = pangolin::TimeNow();
      std::cout << "GetImage time: " << 1000*pangolin::TimeDiff_s(start, end) << "ms" << std::endl;
      start = pangolin::TimeNow();
      std::memcpy(image, lImage->GetDataPointer(), size_bytes);
      end = pangolin::TimeNow();
      std::cout << "memcpy time: " << 1000*pangolin::TimeDiff_s(start, end) << "ms" << std::endl;
      start = pangolin::TimeNow();
      // Required frame properties
      frame_properties[PANGO_CAPTURE_TIME_US] = json::value(lBuffer->GetTimestamp());
      frame_properties[PANGO_HOST_RECEPTION_TIME_US] = json::value(lBuffer->GetReceptionTime());
      end = pangolin::TimeNow();
      std::cout << "Frame properties time: " << 1000*pangolin::TimeDiff_s(start, end) << "ms" << std::endl;

      start = pangolin::TimeNow();
      // Optional frame properties
      if(lTemperatureCelcius != 0) {
          double val;
          PvResult lResult = lTemperatureCelcius->GetValue(val);
          if(lResult.IsSuccess()) {
              frame_properties[PANGO_SENSOR_TEMPERATURE_C] = json::value(val);
          } else {
              pango_print_error("DeviceTemperatureCelsius %f fail\n", val);
          }
      }
      end = pangolin::TimeNow();
      std::cout << "temperature time: " << 1000*pangolin::TimeDiff_s(start, end) << "ms" << std::endl;
      return true;
  } else {
      return false;
  }

}

bool PleoraVideo::GrabNext( unsigned char* image, bool wait)
{
    pangolin::basetime start, now, last;
    const uint32_t timeout = wait ? 1000 : 0;
    bool good = false;
    start = pangolin::TimeNow();
    last = start;
    if(stand_alone_grab_thread) {
        grabbedBuffListMtx.lock();
        int ve = get_num_valid_elements(lGrabbedBuffList);
        if(ve==0) {
            grabbedBuffListMtx.unlock();
            now = pangolin::TimeNow();
            std::cout << "Empty buffer list: " << 1000*pangolin::TimeDiff_s(last, now) << "ms" << std::endl;
            return false;
        }
        std::cout << "Grab next stand alone thread: " << ve << " frames valid, queue size " << lGrabbedBuffList.size() << " popping head" << std::endl;

        GrabbedBufferList::iterator front = lGrabbedBuffList.begin();
        while(!front->valid) {
            ++front;
        }
        grabbedBuffListMtx.unlock();
        now = pangolin::TimeNow();
        std::cout << "Grabbing it to next frame mtx lock: " << 1000*pangolin::TimeDiff_s(last, now) << "ms" << std::endl;
        last = now;

        if ( front->res.IsOK() ) {
            good = ParseBuffer(front->buff, image);
        }
        now = pangolin::TimeNow();
        std::cout << "ParseBuffer: " << 1000*pangolin::TimeDiff_s(last, now) << "ms" << std::endl;
        last = now;

        grabbedBuffListMtx.lock();
        // Flag frame as used, so that it will get released.
        front->valid = false;
        grabbedBuffListMtx.unlock();
        now = pangolin::TimeNow();
        std::cout << "Ivalidating GrabbedBuffer: " << 1000*pangolin::TimeDiff_s(last, now) << "ms" << std::endl;
        last = now;
    } else {
        PvResult lOperationResult;
        PvBuffer *lBuffer = NULL;
        std::cout << "Grab next: " << std::endl;
        // Retrieve next buffer
        const PvResult lResult = lStream->RetrieveBuffer( &lBuffer, &lOperationResult, timeout);
        if ( !lResult.IsOK() ) {
            if(wait || (lResult && !(lResult.GetCode() == PvResult::Code::TIMEOUT))) {
                pango_print_warn("Pleora error: %s,\n'%s'\n", lResult.GetCodeString().GetAscii(), lResult.GetDescription().GetAscii() );
            }
            std::cout << "not OK " << std::endl;
            return false;
        }
        now = pangolin::TimeNow();
        std::cout << "RetrieveBuffer: " << 1000*pangolin::TimeDiff_s(last, now) << "ms" << std::endl;
        last = now;

        if ( lOperationResult.IsOK() ) {
            good = ParseBuffer(lBuffer, image);
        }
        now = pangolin::TimeNow();
        std::cout << "ParseBuffer: " << 1000*pangolin::TimeDiff_s(last, now) << "ms" << std::endl;
        last = now;

        lStream->QueueBuffer( lBuffer );
        now = pangolin::TimeNow();
        std::cout << "QueueBuffer: " << 1000*pangolin::TimeDiff_s(last, now) << "ms" << std::endl;
        last = now;
    }
    now = pangolin::TimeNow();
    std::cout << "Total inner grab: " << 1000*pangolin::TimeDiff_s(start, now) << "ms" << std::endl;
    std::cout << "good="<<good<<std::endl;
    return good;
}


bool PleoraVideo::GrabNewest( unsigned char* image, bool wait )
{
    const uint32_t timeout = wait ? 0xFFFFFFFF : 0;
    bool good = false;
    pangolin::basetime start,last,now;

    if(stand_alone_grab_thread) {
        grabbedBuffListMtx.lock();
        int ve = get_num_valid_elements(lGrabbedBuffList);
        if(ve==0) {
            grabbedBuffListMtx.unlock();
            now = pangolin::TimeNow();
            std::cout << "Empty buffer list: " << 1000*pangolin::TimeDiff_s(last, now) << "ms" << std::endl;
            return false;
        }
        std::cout << "Grab newest stand alone thread: " << ve << " valid frames in queue queue sieze " << lGrabbedBuffList.size() << std::endl;

        GrabbedBufferList::iterator lItOneButLast = lGrabbedBuffList.end();
        --lItOneButLast;
        for(GrabbedBufferList::iterator lIt = lGrabbedBuffList.begin(); lIt != lItOneButLast; ++lIt) {
            //Mark old buffers as invalid so that they can be reclaimed.
            if(lIt->valid) {
                lIt->valid = false;
                std::cout << "marked 1 frame as invalid" << std::endl;
            }
        }
        now = pangolin::TimeNow();
        std::cout << "Flagging old frames as invalid: " << 1000*pangolin::TimeDiff_s(last, now) << "ms" << std::endl;
        last = now;

        GrabbedBufferList::iterator newest = --(lGrabbedBuffList.end());
        grabbedBuffListMtx.unlock();

        if ( newest->res.IsOK() ) {
            good = ParseBuffer(newest->buff, image);
        }
        now = pangolin::TimeNow();
        std::cout << "ParseBuffer: " << 1000*pangolin::TimeDiff_s(last, now) << "ms" << std::endl;
        last = now;
        grabbedBuffListMtx.lock();
        // Flag frame as used, so that it will get released.
        newest->valid = false;
        grabbedBuffListMtx.unlock();
        now = pangolin::TimeNow();
        std::cout << "Ivalidating GrabbedBuffer: " << 1000*pangolin::TimeDiff_s(last, now) << "ms" << std::endl;
        last = now;
    } else {
        PvBuffer *lBuffer0 = NULL;
        PvBuffer *lBuffer = NULL;
        PvResult lOperationResult;
        std::cout << "Grab newest: " << std::endl;

        PvResult lResult = lStream->RetrieveBuffer( &lBuffer, &lOperationResult, timeout );
        if ( !lResult.IsOK() ) {
            if(wait || (lResult && !(lResult.GetCode() == PvResult::Code::TIMEOUT))) {
                pango_print_warn("Pleora error: %s,\n'%s'\n", lResult.GetCodeString().GetAscii(), lResult.GetDescription().GetAscii() );
            }
            std::cout << "Not Ok" << std::endl;
            return false;
        } else if( !lOperationResult.IsOK() ) {
            pango_print_warn("Pleora error: %s,\n'%s'\n", lOperationResult.GetCodeString().GetAscii(), lResult.GetDescription().GetAscii() );
            lStream->QueueBuffer( lBuffer );
            std::cout << "Pleora Error" << std::endl;
            return false;
        }

        // We have at least one frame. Capture more until we fail, 0 timeout
        while(true) {
            PvResult lResult = lStream->RetrieveBuffer( &lBuffer0, &lOperationResult, 0 );
            if ( !lResult.IsOK() ) {
                break;
            } else if( !lOperationResult.IsOK() ) {
                lStream->QueueBuffer( lBuffer0 );
                break;
            } else {
                lStream->QueueBuffer( lBuffer );
                lBuffer = lBuffer0;
            }
        }
        now = pangolin::TimeNow();
        std::cout << "RetrieveBuffer: " << 1000*pangolin::TimeDiff_s(last, now) << "ms" << std::endl;
        last = now;

        good = ParseBuffer(lBuffer, image);
        now = pangolin::TimeNow();
        std::cout << "ParseBuffer: " << 1000*pangolin::TimeDiff_s(last, now) << "ms" << std::endl;
        last = now;

        lStream->QueueBuffer( lBuffer );
        now = pangolin::TimeNow();
        std::cout << "QueueBuffer: " << 1000*pangolin::TimeDiff_s(last, now) << "ms" << std::endl;
        last = now;
    }
    now = pangolin::TimeNow();
    std::cout << "Total inner grab: " << 1000*pangolin::TimeDiff_s(start, now) << "ms" << std::endl;
    std::cout << "good="<<good<<std::endl;
    return good;
}

int64_t PleoraVideo::GetGain()
{
    int64_t val;
    if(lAnalogGain) {
        ThrowOnFailure( lAnalogGain->GetValue(val) );
    }
    return val;
}

void PleoraVideo::SetGain(int64_t val)
{
    if(val >= 0 && lAnalogGain && lAnalogGain->IsWritable()) {
        ThrowOnFailure( lAnalogGain->SetValue(val) );
        frame_properties[PANGO_ANALOG_GAIN] = json::value(val);
    }
}

int64_t PleoraVideo::GetAnalogBlackLevel()
{
    int64_t val;
    if(lAnalogGain) {
        ThrowOnFailure( lAnalogBlackLevel->GetValue(val) );
    }
    return val;
}

void PleoraVideo::SetAnalogBlackLevel(int64_t val)
{
    if(val >= 0 && lAnalogBlackLevel&& lAnalogBlackLevel->IsWritable()) {
        ThrowOnFailure( lAnalogBlackLevel->SetValue(val) );
        frame_properties[PANGO_ANALOG_BLACK_LEVEL] = json::value(val);
    }
}

double PleoraVideo::GetExposure()
{
    double val;
    if( lExposure ) {
        ThrowOnFailure( lExposure->GetValue(val));
    }
    return val;
}

void PleoraVideo::SetExposure(double val)
{
    if(val > 0 && lExposure && lExposure->IsWritable() ) {
        ThrowOnFailure( lExposure->SetValue(val) );
        frame_properties[PANGO_EXPOSURE_US] = json::value(val);
    }
}

//use 0,0,1 for line0 hardware trigger.
//use 2,252,0 for software continuous
void PleoraVideo::SetupTrigger(bool triggerActive, int64_t triggerSource, int64_t acquisitionMode)
{
    if(lAquisitionMode && lTriggerSource && lTriggerMode &&
        lAquisitionMode->IsWritable() && lTriggerSource->IsWritable() && lTriggerMode->IsWritable() ) {
        // Check input is valid.
        const PvGenEnumEntry* entry_src;
        const PvGenEnumEntry* entry_acq;
        lTriggerSource->GetEntryByValue(triggerSource, &entry_src);
        lAquisitionMode->GetEntryByValue(acquisitionMode, &entry_acq);

        if(entry_src && entry_acq) {
            ThrowOnFailure(lTriggerMode->SetValue(triggerActive ? 1 : 0));
            if(triggerActive) {
                pango_print_debug("Pleora: external trigger active\n");
                ThrowOnFailure(lTriggerSource->SetValue(triggerSource));
                ThrowOnFailure(lAquisitionMode->SetValue(acquisitionMode));
            }
        }else{
            pango_print_error("Bad values for trigger options.");
        }
    }
}

template<typename T>
T PleoraVideo::DeviceParam(const char* name)
{
    return GetParam<T>(lDeviceParams, name);
}

template<typename T>
bool PleoraVideo::SetDeviceParam(const char* name, T val)
{
    return SetParam<T>(lDeviceParams, name, val);
}

template<typename T>
T PleoraVideo::StreamParam(const char* name)
{
    return GetParam<T>(lStreamParams, name);
}

template<typename T>
bool PleoraVideo::SetStreamParam(const char* name, T val)
{
    return SetParam<T>(lStreamParams, name, val);
}

}
