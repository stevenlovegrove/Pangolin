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
    }else if( !spfmt.compare("Mono10p") ) {
        return VideoFormatFromString("GRAY10");
    }else if( !spfmt.compare("Mono12p") ) {
        return VideoFormatFromString("GRAY12");
    }else{
        throw VideoException("Unknown Pleora pixel format", spfmt);
    }
}

PleoraVideo::PleoraVideo(const char* model_name, const char* serial_num, size_t index, size_t bpp,  size_t binX, size_t binY, size_t buffer_count,
                         size_t desired_size_x, size_t desired_size_y, size_t desired_pos_x, size_t desired_pos_y, int again, double exposure, bool ext_trig)
    : size_bytes(0), lPvSystem(0), lDevice(0), lStream(0), lDeviceParams(0), lStart(0), lStop(0), lStreamParams(0)
{
    lPvSystem = new PvSystem();
    if ( !lPvSystem ) {
        throw pangolin::VideoException("Pleora: Unable to create PvSystem");
    }

    const PvDeviceInfo *lDeviceInfo = SelectDevice(*lPvSystem, model_name, serial_num, index);
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

    lStream = PvStream::CreateAndOpen( lDeviceInfo->GetConnectionID(), &lResult );
    if ( !lStream ) {
        lDevice->Disconnect();
        PvDevice::Free(lDevice);
        delete lPvSystem;
        throw pangolin::VideoException("Pleora: Unable to open stream", lResult.GetDescription().GetAscii() );
    }

    lDeviceParams = lDevice->GetParameters();
    lStart = dynamic_cast<PvGenCommand*>( lDeviceParams->Get( "AcquisitionStart" ) );
    lStop = dynamic_cast<PvGenCommand*>( lDeviceParams->Get( "AcquisitionStop" ) );

    if( bpp == 8) {
        lDeviceParams->SetEnumValue("PixelFormat", PvString("Mono8") );
    }else if(bpp == 10) {
        lDeviceParams->SetEnumValue("PixelFormat", PvString("Mono10p") );
    }else if(bpp == 12) {
        lDeviceParams->SetEnumValue("PixelFormat", PvString("Mono12p") );
    }


    // Device / Stream property handles
    lDeviceParams = lDevice->GetParameters();
    lStreamParams = lStream->GetParameters();

    // Get Handles to properties we'll be using.
    lAnalogGain = lDeviceParams->GetInteger("AnalogGain");
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
    }catch(std::runtime_error e) {
        pango_print_error("Binning: %s\n", e.what());
    }

    // Height and width will fail if not multiples of 8.
    if(desired_size_x || desired_size_y) {
        try {
            SetDeviceParam<int64_t>("Width",  desired_size_x);
            SetDeviceParam<int64_t>("Height", desired_size_y);
        }catch(std::runtime_error e) {
            pango_print_error("SetSize: %s\n", e.what());

            int64_t max, min;
            lDeviceParams->GetIntegerRange("Width", max, min );
            desired_size_x = max;
            lDeviceParams->GetIntegerRange("Height", max, min );
            desired_size_y = max;

            try {
                SetDeviceParam<int64_t>("Width",  desired_size_x);
                SetDeviceParam<int64_t>("Height", desired_size_y);
            }catch(std::runtime_error e) {
                pango_print_error("Set Full frame: %s\n", e.what());
            }
        }
    }

    // Get actual width and height
    const int w = DeviceParam<int64_t>("Width");
    const int h = DeviceParam<int64_t>("Height");

    // Attempt to set offset
    try{
        SetDeviceParam<int64_t>("OffsetX", desired_pos_x);
        SetDeviceParam<int64_t>("OffsetY", desired_pos_y);
    }catch(std::runtime_error e)
    {
        pango_print_error("Set Offset: %s\n", e.what());
    }


    // Attempt to set AnalogGain, Offset
    try{
        SetGain(again);
        SetExposure(exposure);
    }catch(std::runtime_error e)
    {
        pango_print_error("Set Exposure / Gain: %s\n", e.what());
    }

    // Attempt to set Triggering
    try{
        SetupTrigger(ext_trig, 0, 0);
    }catch(std::runtime_error e)
    {
        pango_print_error("Set Trigger: %s\n", e.what());
    }

    // Reading payload size from device
    const uint32_t lSize = lDevice->GetPayloadSize();

    // Use buffer_count or the maximum number of buffers, whichever is smaller
    const uint32_t lBufferCount = ( lStream->GetQueuedBufferMaximum() < buffer_count ) ?
        lStream->GetQueuedBufferMaximum() :
        buffer_count;

    // Allocate buffers and queue
    for ( uint32_t i = 0; i < lBufferCount; i++ )
    {
        PvBuffer *lBuffer = new PvBuffer;
        lBuffer->Alloc( static_cast<uint32_t>( lSize ) );
        lBufferList.push_back( lBuffer );
    }

    // Setup pangolin for stream
    PvGenEnum* lpixfmt = dynamic_cast<PvGenEnum*>( lDeviceParams->Get("PixelFormat") );
    const VideoPixelFormat fmt = PleoraFormat(lpixfmt);
    streams.push_back(StreamInfo(fmt, w, h, (w*fmt.bpp)/8));
    size_bytes = lSize;

    Start();
}

PleoraVideo::~PleoraVideo()
{
    Stop();

    // Free buffers
    for( BufferList::iterator lIt = lBufferList.begin(); lIt != lBufferList.end(); lIt++ ) {
        delete *lIt;
    }

    if(lStream) {
        lStream->Close();
        PvStream::Free( lStream );
    }

    if(lDevice) {
        lDevice->Disconnect();
        PvDevice::Free( lDevice );
    }

    delete lPvSystem;
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
    }else{
        pango_print_warn("PleoraVideo: Already started.\n");
    }
}

void PleoraVideo::Stop()
{
    if(lStream->GetQueuedBufferCount() > 0) {
        lStop->Execute();
        lDevice->StreamDisable();

        // Abort all buffers from the stream and dequeue
        lStream->AbortQueuedBuffers();
        while ( lStream->GetQueuedBufferCount() > 0 )
        {
            PvBuffer *lBuffer = NULL;
            PvResult lOperationResult;
            lStream->RetrieveBuffer( &lBuffer, &lOperationResult );
        }
    }else{
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

bool PleoraVideo::GrabNext( unsigned char* image, bool wait)
{
    PvBuffer *lBuffer = NULL;
    PvResult lOperationResult;
    const uint32_t timeout = wait ? 1000 : 0;

    // Retrieve next buffer
    PvResult lResult = lStream->RetrieveBuffer( &lBuffer, &lOperationResult, timeout);
    if ( !lResult.IsOK() ) {
        if(wait || (lResult && !(lResult.GetCode() == PvResult::Code::TIMEOUT))) {
            pango_print_warn("Pleora error: %s,\n'%s'\n", lResult.GetCodeString().GetAscii(), lResult.GetDescription().GetAscii() );
        }
        return false;
    }

    bool good = false;

    if ( lOperationResult.IsOK() )
    {
        PvPayloadType lType = lBuffer->GetPayloadType();
        if ( lType == PvPayloadTypeImage )
        {
            PvImage *lImage = lBuffer->GetImage();
            std::memcpy(image, lImage->GetDataPointer(), size_bytes);
            frame_properties[PANGO_CAPTURE_TIME_US] = json::value(lBuffer->GetTimestamp());
            frame_properties[PANGO_HOST_RECEPTION_TIME_US] = json::value(lBuffer->GetReceptionTime());
			double val;
			PvResult lResult = lDeviceParams->GetFloatValue("DeviceTemperatureCelsius", val);
			if (lResult.IsSuccess()) {
				frame_properties[PANGO_SENSOR_TEMPERATURE_C] = json::value(val);
			} else {
				pango_print_error("DeviceTemperatureCelsius %f fail\n", val);
			}
            good = true;
        }
    } else {
        pango_print_warn("Pleora error: %s,\n'%s'\n", lResult.GetCodeString().GetAscii(), lResult.GetDescription().GetAscii() );
    }

    lStream->QueueBuffer( lBuffer );
    return good;
}

bool PleoraVideo::GrabNewest( unsigned char* image, bool wait )
{
    PvBuffer *lBuffer0 = NULL;
    PvBuffer *lBuffer = NULL;
    PvResult lOperationResult;
    const uint32_t timeout = wait ? 0xFFFFFFFF : 0;

    PvResult lResult = lStream->RetrieveBuffer( &lBuffer, &lOperationResult, timeout );
    if ( !lResult.IsOK() ) {
        if(wait || (lResult && !(lResult.GetCode() == PvResult::Code::TIMEOUT))) {
            pango_print_warn("Pleora error: %s,\n'%s'\n", lResult.GetCodeString().GetAscii(), lResult.GetDescription().GetAscii() );
        }
        return false;
    }else if( !lOperationResult.IsOK() ) {
        pango_print_warn("Pleora error: %s,\n'%s'\n", lOperationResult.GetCodeString().GetAscii(), lResult.GetDescription().GetAscii() );
        lStream->QueueBuffer( lBuffer );
        return false;
    }

    // We have at least one frame. Capture more until we fail, 0 timeout
    while(true) {
        PvResult lResult = lStream->RetrieveBuffer( &lBuffer0, &lOperationResult, 0 );
        if ( !lResult.IsOK() ) {
            break;
        }else if( !lOperationResult.IsOK() ) {
            lStream->QueueBuffer( lBuffer0 );
            break;
        }else{
            lStream->QueueBuffer( lBuffer );
            lBuffer = lBuffer0;
        }
    }

    bool good = false;

    PvPayloadType lType = lBuffer->GetPayloadType();
    if ( lType == PvPayloadTypeImage )
    {
        PvImage *lImage = lBuffer->GetImage();
        std::memcpy(image, lImage->GetDataPointer(), size_bytes);

        // Required frame properties
        frame_properties[PANGO_CAPTURE_TIME_US] = json::value(lBuffer->GetTimestamp());
        frame_properties[PANGO_HOST_RECEPTION_TIME_US] = json::value(lBuffer->GetReceptionTime());

        // Optional frame properties
        if(lTemperatureCelcius) {
            double val;
            PvResult lResult = lTemperatureCelcius->GetValue(val);
            if(lResult.IsSuccess()) {
                frame_properties[PANGO_SENSOR_TEMPERATURE_C] = json::value(val);
            } else {
                pango_print_error("DeviceTemperatureCelsius %f fail\n", val);
            }
        }
        good = true;
    }

    lStream->QueueBuffer( lBuffer );
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

double PleoraVideo::GetExposure() {
    double val;
    if( lExposure ) {
        ThrowOnFailure( lExposure->GetValue(val));
    }
    return val;
}

void PleoraVideo::SetExposure(double val) {
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
        lAquisitionMode->IsWritable() && lTriggerSource->IsWritable() && lTriggerMode->IsWritable() )
    {
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
