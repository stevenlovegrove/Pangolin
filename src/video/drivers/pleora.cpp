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
        throw std::runtime_error(res.GetCodeString().GetAscii());
    }
    return ret;
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

PleoraVideo::PleoraVideo(const char* model_name, const char* serial_num, size_t index, size_t bpp,  size_t binX, size_t binY, size_t buffer_count)
    : lPvSystem(0), lDevice(0), lStream(0)
{
    lPvSystem = new PvSystem();
    if ( !lPvSystem ) {
        throw pangolin::VideoException("Pleora: Unable to create PvSystem");
    }

    const PvDeviceInfo *lDeviceInfo = SelectDevice(*lPvSystem, model_name, serial_num, index);
    if ( !lDeviceInfo ) {
        throw pangolin::VideoException("Pleora: Unable to select device");
    }

    PvResult lResult;
    lDevice = PvDevice::CreateAndConnect( lDeviceInfo, &lResult );
    if ( !lDevice ) {
        throw pangolin::VideoException("Pleora: Unable to connect to device", lResult.GetDescription().GetAscii() );
    }

    lStream = PvStream::CreateAndOpen( lDeviceInfo->GetConnectionID(), &lResult );
    if ( !lStream ) {
        lDevice->Disconnect();
        PvDevice::Free(lDevice);
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


    lResult =lDeviceParams->SetIntegerValue("BinningHorizontal", binX );
    if(lResult.IsFailure()){
        pango_print_error("BinningHorizontal %zu fail\n", binX);
    }
    lResult =lDeviceParams->SetIntegerValue("BinningVertical", binY );
    if(lResult.IsFailure()){
        pango_print_error("BinningVertical %zu fail\n", binY);
    }

    lStreamParams = lStream->GetParameters();

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

    const int w = DeviceParam<int64_t>("Width");
    const int h = DeviceParam<int64_t>("Height");

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
    // Queue all buffers in the stream
    for( BufferList::iterator lIt = lBufferList.begin(); lIt != lBufferList.end(); lIt++ ) {
        lStream->QueueBuffer( *lIt );
    }

    lDevice->StreamEnable();
    lStart->Execute();
}

void PleoraVideo::Stop()
{
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
}

size_t PleoraVideo::SizeBytes() const
{
    return size_bytes;
}

const std::vector<StreamInfo>& PleoraVideo::Streams() const
{
    return streams;
}

bool PleoraVideo::GrabNext( unsigned char* image, bool /*wait*/ )
{
    PvBuffer *lBuffer = NULL;
    PvResult lOperationResult;

    // Retrieve next buffer
    PvResult lResult = lStream->RetrieveBuffer( &lBuffer, &lOperationResult, 1000 );
    if ( !lResult.IsOK() ) {
        pango_print_warn("Pleora error: %s\n", lResult.GetCodeString().GetAscii() );
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
            good = true;
        }
    } else {
        pango_print_warn("Pleora error: %s\n", lOperationResult.GetCodeString().GetAscii() );
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
        pango_print_warn("Pleora error: %s\n", lResult.GetCodeString().GetAscii() );
        return false;
    }else if( !lOperationResult.IsOK() ) {
        pango_print_warn("Pleora error: %s\n", lOperationResult.GetCodeString().GetAscii() );
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
        good = true;
    }

    lStream->QueueBuffer( lBuffer );
    return good;
}

template<typename T>
T PleoraVideo::DeviceParam(const char* name)
{
    return GetParam<T>(lDeviceParams, name);
}

template<typename T>
T PleoraVideo::StreamParam(const char* name)
{
    return GetParam<T>(lStreamParams, name);
}

}
