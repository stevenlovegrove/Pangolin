#ifndef NOMINMAX
#  define NOMINMAX
#endif

#include <mfapi.h>
#include <mferror.h>
#include <mfidl.h>
#include <mfreadwrite.h>

#include <dshow.h>
#include <ks.h>
#include <ksmedia.h>
#include <ksproxy.h>
#include <vidcap.h>

#include <pangolin/factory/factory_registry.h>
#include <pangolin/utils/timer.h>
#include <pangolin/video/drivers/uvc_mediafoundation.h>
#include <pangolin/video/iostream_operators.h>

#include <condition_variable>
#include <mutex>

namespace pangolin
{

static constexpr DWORD KS_CONTROL_NODE_ID_INVALID = ~0;

const GUID GUID_EXTENSION_UNIT_DESCRIPTOR_OV580{
  0x2ccb0bda, 0x6331, 0x4fdb, 0x85, 0x0e, 0x79, 0x05, 0x4d, 0xbd, 0x56, 0x71
};

class AsyncSourceReader : public IMFSourceReaderCallback
{
public:
    AsyncSourceReader(IMFMediaSource* mediaSource, uint64_t timeout_ms) : ref_(1), timeout_(std::chrono::milliseconds(timeout_ms))
    {
        HRESULT hr;

        IMFAttributes *pAttributes;
        hr = MFCreateAttributes(&pAttributes, 1);
        if (FAILED(hr))
        {
            throw VideoException(FormatString("Unable to initialize attributes for source reader (%)", hr));
        }

        hr = pAttributes->SetUnknown(MF_SOURCE_READER_ASYNC_CALLBACK, this);
        if (FAILED(hr))
        {
            throw VideoException(FormatString("Unable to set async attribute for source reader (%)", hr));
        }

        hr = MFCreateSourceReaderFromMediaSource(mediaSource, pAttributes, &reader_);
        if(FAILED(hr))
        {
            throw VideoException(FormatString("Unable to create source reader from UVC media source (%)", hr));
        }
    }

    // IUnknown methods
    STDMETHODIMP QueryInterface(REFIID iid, void** ppv)
    {
      if (iid != __uuidof(IMFSourceReaderCallback)) {
        return E_NOINTERFACE;
      }
      InterlockedIncrement(&ref_);
      *ppv = this;
      return S_OK;
    }
    STDMETHODIMP_(ULONG) AddRef()
    {
        return InterlockedIncrement(&ref_);
    }
    STDMETHODIMP_(ULONG) Release()
    {
        ULONG snapped = InterlockedDecrement(&ref_);
        if (snapped == 0)
        {
            delete this;
        }
        return snapped;
    }

    // IMFSourceReaderCallback methods
    STDMETHODIMP OnReadSample(HRESULT hrStatus, DWORD dwStreamIndex, DWORD dwStreamFlags, LONGLONG llTimestamp, IMFSample *pSample)
    {
        {
            std::lock_guard<std::mutex> lock(sample_lock_);

            sample_hr_ = hrStatus;
            sample_actual_stream_index_ = dwStreamIndex;
            sample_stream_flags_ = dwStreamFlags;
            sample_timestamp_ = llTimestamp;
            sample_ = pSample;

            if (sample_ != nullptr)
            {
                sample_->AddRef();
            }
        }

        sample_ready_.notify_one();

        return S_OK;
    }

    STDMETHODIMP OnEvent(DWORD, IMFMediaEvent *)
    {
        return S_OK;
    }

    STDMETHODIMP OnFlush(DWORD)
    {
        return S_OK;
    }

    IMFSourceReader* AddRefReader()
    {
        reader_->AddRef();
        return reader_;
    }

    HRESULT ReadSample(DWORD dwStreamIndex, DWORD dwControlFlags, DWORD* pdwActualStreamIndex, DWORD* pdwStreamFlags, LONGLONG* pllTimestamp, IMFSample** ppSample)
    {
      auto wait_target = std::chrono::steady_clock::now() + timeout_;

      std::unique_lock<std::mutex> lock(sample_lock_);

      // If there's already a pending read, just piggyback on it and wait. Otherwise, issue now.
      if (sample_hr_ != E_PENDING)
      {
        HRESULT sync_hr = reader_->ReadSample(dwStreamIndex, dwControlFlags, NULL, NULL, NULL, NULL);
        if (FAILED(sync_hr))
        {
            return sync_hr;
        }

        sample_hr_ = E_PENDING;
      }

      while (sample_hr_ == E_PENDING)
      {
          if (sample_ready_.wait_until(lock, wait_target) == std::cv_status::timeout)
          {
             // Leave sample_hr_ as E_PENDING since there's still an oustanding async call.
             // It may be set later by the callback.
             return RPC_E_TIMEOUT;
          }
      }

      if (SUCCEEDED(sample_hr_))
      {
          *pdwActualStreamIndex = sample_actual_stream_index_;
          *pdwStreamFlags = sample_stream_flags_;
          *pllTimestamp = sample_timestamp_;

          *ppSample = sample_;
          sample_ = nullptr;
      }

      return sample_hr_;
    }

private:
    // Destructor is private. Caller should call Release.
    virtual ~AsyncSourceReader()
    {
        if (reader_ != nullptr)
        {
            reader_->Release();
        }

        if (sample_ != nullptr)
        {
            sample_->Release();
        }
    }

    LONG ref_;
    std::chrono::steady_clock::duration timeout_;
    IMFSourceReader* reader_;

    std::mutex sample_lock_;
    std::condition_variable sample_ready_;
    // This just needs to be initialized to anything other than E_PENDING.
    HRESULT sample_hr_ = E_UNEXPECTED;
    DWORD sample_actual_stream_index_;
    DWORD sample_stream_flags_;
    LONGLONG sample_timestamp_;
    IMFSample* sample_ = nullptr;
};

UvcMediaFoundationVideo::UvcMediaFoundationVideo(int vendorId, int productId, const std::string& instanceId, size_t width, size_t height, int fps)
    : size_bytes(0),
      mediaSource(nullptr),
      sourceReader(nullptr),
      baseFilter(nullptr),
      ksControl(nullptr),
      ksControlNodeId(KS_CONTROL_NODE_ID_INVALID),
      camera_control(nullptr),
      video_control(nullptr),
      expected_fps(fps)
{
	HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if(FAILED(hr))
    {
        throw VideoException("CoInitializeEx failed");
    }

    if(FAILED(MFStartup(MF_VERSION)))
    {
        throw VideoException("MfStartup failed");
    }

    if(!FindDevice(vendorId, productId, instanceId))
    {
        throw VideoException("Unable to open UVC media source");
    }
    // Use async always (same interface, but allows for timing out).
    InitDevice(width, height, true);

    device_properties[PANGO_HAS_TIMING_DATA] = true;
}

UvcMediaFoundationVideo::~UvcMediaFoundationVideo()
{
    DeinitDevice();
    HRESULT hr = MFShutdown();
    if(FAILED(hr))
    {
        pango_print_warn("MFShutdown failed with result %X", hr);
    }
    CoUninitialize();
}

void UvcMediaFoundationVideo::Start()
{

}

void UvcMediaFoundationVideo::Stop()
{
}

size_t UvcMediaFoundationVideo::SizeBytes() const
{
    return size_bytes;
}

const std::vector<pangolin::StreamInfo>& UvcMediaFoundationVideo::Streams() const
{
    return streams;
}


bool UvcMediaFoundationVideo::GrabNext(unsigned char* image, bool wait)
{
    HRESULT hr;
    IMFSample* sample = nullptr;
    DWORD streamIndex = 0;
    DWORD flags = 0;
    LONGLONG timeStamp;

    if (asyncSourceReader != nullptr)
    {
        hr = asyncSourceReader->ReadSample(
                (DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM, 0, &streamIndex, &flags, &timeStamp, &sample);
    }
    else
    {
        hr = sourceReader->ReadSample(
                (DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM, 0, &streamIndex, &flags, &timeStamp, &sample);
    }

    if(SUCCEEDED(hr))
    {
        if((flags & MF_SOURCE_READERF_ENDOFSTREAM) != 0)
        {
            return false;
        }

        if((flags & MF_SOURCE_READERF_STREAMTICK) != 0)
        {
            return false;
        }
        // Believe that at this point sample should not be null
        if(!sample)
        {
            return false;
        }
    }
    else if (hr == MF_E_VIDEO_RECORDING_DEVICE_INVALIDATED && wait)
    {
      std::this_thread::sleep_for(std::chrono::microseconds(1000000 / expected_fps));
    }

    IMFMediaBuffer* mediaBuffer = nullptr;
    if(SUCCEEDED(hr))
    {
        hr = sample->ConvertToContiguousBuffer(&mediaBuffer);
    }
    if(SUCCEEDED(hr))
    {
        // Use the 2D buffer interface if it's available
        IMF2DBuffer* mediaBuffer2d = nullptr;
        hr = mediaBuffer->QueryInterface(&mediaBuffer2d);
        if(SUCCEEDED(hr))
        {
            hr = mediaBuffer2d->ContiguousCopyTo(image, (DWORD)size_bytes);
            mediaBuffer2d->Release();
            mediaBuffer2d = nullptr;
        }
        else
        {
            // No 2D buffer is available
            byte* buffer;
            DWORD bufferSize = 0;
            hr = mediaBuffer->Lock(&buffer, nullptr, &bufferSize);
            if(SUCCEEDED(hr))
            {
                size_t copySize = std::min((size_t)bufferSize, size_bytes);
                memcpy(image, buffer, copySize);
                mediaBuffer->Unlock();
            }
        }
    }

    if(SUCCEEDED(hr))
    {
        using namespace std::chrono;
        frame_properties[PANGO_HOST_RECEPTION_TIME_US] = picojson::value(pangolin::Time_us(pangolin::TimeNow()));
    }

    if(mediaBuffer)
    {
        mediaBuffer->Release();
        mediaBuffer = nullptr;
    }

    if(sample)
    {
        sample->Release();
        sample = nullptr;
    }

    return SUCCEEDED(hr);
}

bool UvcMediaFoundationVideo::GrabNewest(unsigned char* image, bool wait)
{
    return GrabNext(image, wait);
}

int UvcMediaFoundationVideo::IoCtrl(uint8_t unit, uint8_t ctrl, unsigned char* data, int len, UvcRequestCode req_code)
{
    if(!ksControl || ksControlNodeId == KS_CONTROL_NODE_ID_INVALID)
    {
        return -1;
    }

    HRESULT hr;
    KSP_NODE s = {};
    ULONG ulBytesReturned;

    s.Property.Set = GUID_EXTENSION_UNIT_DESCRIPTOR_OV580;
    s.Property.Id = ctrl;
    s.NodeId = ksControlNodeId;

    s.Property.Flags = KSPROPERTY_TYPE_TOPOLOGY;
    if(req_code == UVC_GET_CUR)
    {
        s.Property.Flags |= KSPROPERTY_TYPE_GET;
    }
    else if(req_code == UVC_SET_CUR)
    {
        s.Property.Flags |= KSPROPERTY_TYPE_SET;
    }
    else if(req_code == UVC_GET_LEN)
    {
        if(len != sizeof(uint16_t)){
            std::cerr << "UVC_GET_LEN requested with a buffer not the size of uint16_t" << std::endl;
            return -1;
        }

        s.Property.Flags |= KSPROPERTY_TYPE_GET;
        hr = ksControl->KsProperty((PKSPROPERTY)&s, sizeof(s), NULL, 0, &ulBytesReturned);
        if(hr == HRESULT_FROM_WIN32(ERROR_MORE_DATA))
        {
            *(uint16_t*)data = (uint16_t)ulBytesReturned;
            return 0;
        }
        return -1;
    }

    hr = ksControl->KsProperty((PKSPROPERTY)&s, sizeof(s), data, len, &ulBytesReturned);

    if (hr == HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER))
    {
      std::cerr << "IoCtrl result is " << ulBytesReturned << ", buffer supplied is " << len << ", result truncated." << std::endl;
      hr = S_OK;
    }

    if(FAILED(hr))
    {
      pango_print_error("KsProperty failed on UVC device");
      return -1;
    }

    return 0;
}

// https://technet.microsoft.com/en-us/dd318253(v=vs.90)
// Windows exposure time is specified in log base 2
long to_100micros(long v)
{
    double res = pow(2.0, v);
    return static_cast<long>(res * 10000);
}

long from_100micros(long val)
{
    double d = val * 0.0001;
    double l = (d != 0) ? std::log2(d) : 1;
    long v = static_cast<long>(std::roundl(l));
    return v;
}

bool UvcMediaFoundationVideo::GetExposure(int& exp_us)
{
    long val = 0;
    long flags = 0;

    if(!camera_control)
    {
        pango_print_warn("GetExposure called with no camera_control interface");
        return false;
    }
    HRESULT hr = camera_control->Get(CameraControl_Exposure, &val, &flags);

    exp_us = to_100micros(val) * 100;
    return SUCCEEDED(hr);
}

bool UvcMediaFoundationVideo::SetExposure(int exp_us)
{
    if(!camera_control)
    {
        pango_print_warn("SetExposure called with no camera_control interface");
        return false;
    }
    HRESULT hr = camera_control->Set(CameraControl_Exposure, from_100micros(exp_us / 100), CameraControl_Flags_Manual);
    return SUCCEEDED(hr);
}

bool UvcMediaFoundationVideo::GetGain(float& gain)
{
    if(!video_control)
    {
        pango_print_warn("GetGain called without gain controlls populated");
        return false;
    }

    long camGain;
    long flags;
    HRESULT hr = video_control->Get(VideoProcAmp_Gain, &camGain, &flags);

    if(FAILED(hr))
    {
        pango_print_warn("Failed to read the gain. hr:0x%x", hr);
        return false;
    }

    // (FW bug) We can't rely on gainCamMin/gainCamMax here, as they don't correspond to the values returned
    // by this API
    // (HAL bug) This assumes that the caller knows to normalize back into API space
    gain = static_cast<float>(camGain);


    return true;
}

bool UvcMediaFoundationVideo::SetGain(float gain)
{
    if(!video_control)
    {
        pango_print_warn("SetGain called without gain controlls populated");
        return false;
    }

    // (FW bug) We can't rely on gainCamMin/gainCamMax here, as they don't correspond to the values supported
    // by this API
    // (HAL bug) This assumes that the caller knows to normalize into camera register space [16, 256]
    long targetGain = gain;

    HRESULT hr = video_control->Set(VideoProcAmp_Gain, targetGain, VideoProcAmp_Flags_Manual);
    if(FAILED(hr))
    {
        pango_print_warn("Failed to set gain value %f. hr:0x%x", gain, hr);
        return false;
    }
    return true;
}


const picojson::value& UvcMediaFoundationVideo::DeviceProperties() const
{
    return device_properties;
}

const picojson::value& UvcMediaFoundationVideo::FrameProperties() const
{
    return frame_properties;
}

bool UvcMediaFoundationVideo::FindDevice(int vendorId, int productId, const std::string& instanceId)
{
    // Create attributes for finding UVC devices
    IMFAttributes* searchAttributes = nullptr;
    HRESULT hr = MFCreateAttributes(&searchAttributes, 1);
    if(FAILED(hr))
    {
        pango_print_error("Unable to create UVC device search attributes");
    }

    // Request video capture devices
    if(SUCCEEDED(hr))
    {
        hr = searchAttributes->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE,
                                       MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);
        if(FAILED(hr))
        {
            pango_print_error("Unable to set UVC source type attribute");
        }
    }

    // Enumerate the devices
    IMFActivate** devices = nullptr;
    UINT32 deviceCount = 0;
    if(SUCCEEDED(hr))
    {
        hr = MFEnumDeviceSources(searchAttributes, &devices, &deviceCount);
        if(FAILED(hr))
        {
            pango_print_error("Unable to enumerate UVC device sources");
        }
    }

    std::wstring symLink;
    bool activatedDevice = false;
    if(SUCCEEDED(hr))
    {
        std::wstring wInstanceId;
        wInstanceId.assign(instanceId.begin(), instanceId.end());

        for(UINT32 i = 0; i < deviceCount; ++i)
        {
            // Get this device's sym link
            WCHAR* symLinkWCStr = nullptr;
            UINT32 symLinkLength = 0;
            hr = devices[i]->GetAllocatedString(
                    MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_SYMBOLIC_LINK, &symLinkWCStr, &symLinkLength);
            if(FAILED(hr))
            {
                hr = S_OK;
                continue;
            }

            std::wstring checkSymLink(symLinkWCStr);

            // Check if this device matches the requested vendor ID and product ID
            if(!DeviceMatches(checkSymLink, vendorId, productId, wInstanceId))
            {
                continue;
            }

            hr = devices[i]->ActivateObject(IID_PPV_ARGS(&mediaSource));
            activatedDevice = SUCCEEDED(hr);
            if(activatedDevice)
            {
                symLink = std::move(checkSymLink);
            }
            break;
        }

        if(!activatedDevice)
        {
            pango_print_error("Unable to activate UVC device source");
            hr = E_FAIL;
        }
    }

    for(UINT32 i = 0; i < deviceCount; ++i)
    {
        devices[i]->Release();
        devices[i] = nullptr;
    }
    devices = nullptr;

    CoTaskMemFree(devices);

    if(searchAttributes != nullptr)
    {
        searchAttributes->Release();
        searchAttributes = nullptr;
    }

    // Find the DirectShow device
    ICreateDevEnum* dshowDevices = nullptr;
    if(SUCCEEDED(hr))
    {
        hr = CoCreateInstance(CLSID_SystemDeviceEnum, 0, CLSCTX_INPROC, IID_ICreateDevEnum, (void**)&dshowDevices);
    }

    IEnumMoniker* videoInputEnumerator = nullptr;
    if(SUCCEEDED(hr))
    {
        hr = dshowDevices->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &videoInputEnumerator, 0);
    }

    if(SUCCEEDED(hr))
    {
        IMoniker* moniker = nullptr;
        while((hr = videoInputEnumerator->Next(1, &moniker, 0)) == S_OK)
        {
            IPropertyBag* propertyBag = nullptr;
            hr = moniker->BindToStorage(0, 0, IID_IPropertyBag, (void**)&propertyBag);
            if(FAILED(hr))
            {
                moniker->Release();
                moniker = nullptr;
                continue;
            }

            VARIANT variantPath;
            VariantInit(&variantPath);
            hr = propertyBag->Read(L"DevicePath", &variantPath, nullptr);

            bool pathMatches;
            if(SUCCEEDED(hr) && variantPath.vt == VT_BSTR)
            {
                // Determine if this is the correct device by comparing its path against the source's symbolic link
                // This breaks the rules, but it seems to be the only way to make sure it's the correct device

                // DirectShow and MediaFoundation appear to each create their own symbolic link which contains a GUID
                // Ignore the GUID portion of the link, leaving just the path information
                size_t braceOffset = symLink.find(L'{');

                pathMatches = 0 == std::wcsncmp(symLink.c_str(), variantPath.bstrVal, braceOffset);
            }
            else
            {
                pathMatches = false;
            }

            VARIANT variantFriendlyName;
            VariantInit(&variantFriendlyName);
            hr = propertyBag->Read(L"FriendlyName", &variantFriendlyName, nullptr);

            if(!pathMatches)
            {
                moniker->Release();
                moniker = nullptr;
                continue;
            }

            // Found the correct video input
            break;
        }

        if(moniker)
        {
            hr = moniker->BindToObject(0, 0, IID_IBaseFilter, (void**)&baseFilter);
        }
        else
        {
            hr = E_FAIL;
        }

        IKsTopologyInfo* ksTopologyInfo = nullptr;
        if(SUCCEEDED(hr))
        {
            hr = baseFilter->QueryInterface(__uuidof(IKsTopologyInfo), (void**)&ksTopologyInfo);
        }

        DWORD numberOfNodes = 0;
        if(SUCCEEDED(hr))
        {
            hr = ksTopologyInfo->get_NumNodes(&numberOfNodes);
        }

        GUID nodeGuid;
        for(DWORD nodeIndex = 0; nodeIndex < numberOfNodes; ++nodeIndex)
        {
            if(FAILED(ksTopologyInfo->get_NodeType(nodeIndex, &nodeGuid)))
            {
                continue;
            }

            if(nodeGuid == KSNODETYPE_DEV_SPECIFIC)
            {
                // This is the extension node
                IKsNodeControl* pUnknown = nullptr;
                hr = ksTopologyInfo->CreateNodeInstance(nodeIndex, __uuidof(IUnknown), (void**)&pUnknown);

                if(SUCCEEDED(hr) && pUnknown != nullptr)
                {
                    hr = pUnknown->QueryInterface(__uuidof(IKsControl), (void**)&ksControl);
                }
                if(SUCCEEDED(hr))
                {
                    ksControlNodeId = nodeIndex;
                }

                if(pUnknown)
                {
                    pUnknown->Release();
                    pUnknown = nullptr;
                }
            }
        }

        if(ksTopologyInfo)
        {
            ksTopologyInfo->Release();
            ksTopologyInfo = nullptr;
        }

        if(moniker)
        {
            moniker->Release();
            moniker = nullptr;
        }
    }

    if(videoInputEnumerator)
    {
        videoInputEnumerator->Release();
    }

    if(dshowDevices)
    {
        dshowDevices->Release();
    }

    return SUCCEEDED(hr);
}


const GUID MFVideoFormat_Y10 =
{
    0x20303159,
    0x0000,
    0x0010,
    0x80,
    0x00,
    0x00,
    0xAA,
    0x00,
    0x38,
    0x9b,
    0x71
};

void UvcMediaFoundationVideo::InitDevice(size_t width, size_t height, bool async)
{
    HRESULT hr;

    if (async)
    {
        int64_t timeout_ms = expected_fps ? 4 * 1000 / expected_fps : 500;
        asyncSourceReader = new AsyncSourceReader(mediaSource, timeout_ms);
        sourceReader = asyncSourceReader->AddRefReader();
    }
    else
    {
        hr = MFCreateSourceReaderFromMediaSource(mediaSource, nullptr, &sourceReader);
        if(FAILED(hr))
        {
            throw VideoException(FormatString("Unable to create source reader from UVC media source (%)", hr));
        }
    }

    // Find the closest supported resolution
    UINT32 stride;
    bool hasValidStride = false;
    PixelFormat pixelFormat;

    uint32_t bit_depth = 0;
    IMFMediaType* bestMediaType = nullptr;
    int bestError = std::numeric_limits<int>::max();
    UINT32 bestWidth;
    UINT32 bestHeight;
    UINT32 bestStride;
    GUID bestGuid;

    for(DWORD i = 0;; ++i)
    {
        IMFMediaType* checkMediaType;
        hr = sourceReader->GetNativeMediaType(MF_SOURCE_READER_FIRST_VIDEO_STREAM, i, &checkMediaType);
        if(FAILED(hr))
        {
            if(hr == MF_E_NO_MORE_TYPES)
            {
                // Reached the end of the available media types
                hr = S_OK;
            }
            else
            {
                pango_print_error("Failed to get UVC native media type");
            }
            break;
        }

        UINT32 checkWidth;
        UINT32 checkHeight;
        if(FAILED(MFGetAttributeSize(checkMediaType, MF_MT_FRAME_SIZE, &checkWidth, &checkHeight)))
        {
            checkWidth = 0;
            checkHeight = 0;
        }

        int checkError = abs(int(checkWidth) - int(width)) + abs(int(checkHeight) - int(height));
        if(bestError > checkError)
        {
            // Release the previous best
            if(bestMediaType)
            {
                bestMediaType->Release();
            }

            bestError = checkError;
            bestMediaType = checkMediaType;
            bestWidth = checkWidth;
            bestHeight = checkHeight;

            if(FAILED(checkMediaType->GetGUID(MF_MT_SUBTYPE, &bestGuid)))
            {
                bestGuid = {0};
            }

            const auto stride_result = checkMediaType->GetUINT32(MF_MT_DEFAULT_STRIDE,&bestStride);
            hasValidStride = !(stride_result == MF_E_ATTRIBUTENOTFOUND);
        }
        else
        {
            checkMediaType->Release();
        }
    }

    if(bestMediaType)
    {
        if(SUCCEEDED(hr))
        {
            sourceReader->SetCurrentMediaType(MF_SOURCE_READER_FIRST_VIDEO_STREAM, nullptr, bestMediaType);
            width = bestWidth;
            height = bestHeight;
            if(hasValidStride)
            {
                stride = bestStride;
            }

            if(bestGuid == MFVideoFormat_YUY2)
            {
                pixelFormat = PixelFormatFromString("GRAY8");
                bit_depth = 8;
            }
            else if(bestGuid == MFVideoFormat_Y10)
            {
                pixelFormat = PixelFormatFromString("GRAY10");
                bit_depth = 10;
            }
            else if((unsigned char)((bestGuid.Data1 >> 0) & 0xff) == 'Y' && (unsigned char)((bestGuid.Data1 >> 8) & 0xff) == '8')
            {
                pixelFormat = PixelFormatFromString("GRAY8");
                bit_depth = 8;
            }
            else
            {
                pango_print_warn("Unexpected MFVideoFormat with FOURCC %c.%c.%c.%c",
                                 (unsigned char)((bestGuid.Data1 >> 0) & 0xff),
                                 (unsigned char)((bestGuid.Data1 >> 8) & 0xff),
                                 (unsigned char)((bestGuid.Data1 >> 16) & 0xff),
                                 (unsigned char)((bestGuid.Data1 >> 24) & 0xff));
            }
        }

        bestMediaType->Release();
    }
    else
    {
        width = 0;
        height = 0;
    }

    if(SUCCEEDED(hr))
    {
        PopulateGainControls();
    }

    if(SUCCEEDED(hr))
    {
        hr = mediaSource->QueryInterface(__uuidof(IAMCameraControl), (void**)&camera_control);
        if(FAILED(hr))
        {
            // On failure, soldier on without manual exposure controls
            pango_print_warn("Failed to populate IAMCameraControl (0x%x). Exposure control will not function", hr);
        }
    }

    if(hasValidStride)
    {
        size_bytes = stride * height;
    }
    else
    {
        size_bytes = width * pixelFormat.bpp * height / 8;
    }
    pixelFormat.channel_bit_depth = bit_depth;
    const StreamInfo stream_info(pixelFormat, width, height, size_bytes / height, 0);
    streams.emplace_back(stream_info);
}

void UvcMediaFoundationVideo::PopulateGainControls()
{
    if(!mediaSource)
    {
        pango_print_warn("PopulateGainControls called without a media source");
        return;
    }

    HRESULT hr = mediaSource->QueryInterface(__uuidof(IAMVideoProcAmp) , (void**)&video_control);

    // read the min and max values from the camera to allow mapping from our fixed ranges
    if(SUCCEEDED(hr))
    {
        long steppingDelta;
        long flags;

        // FW bug: this range is being reported as [0, 1024], but the Get/Set pathway handles register values [16, 256]
        hr = video_control->GetRange(VideoProcAmp_Gain, &gainCamMin, &gainCamMax, &steppingDelta, &gainCamDefault, &flags);

        // should we be checking that flags are correctly set to VideoProcAmp_Flags_Manual ?
    }

    if(FAILED(hr))
    {
        pango_print_warn("Failed to populate gain controls (0x%x)", hr);
        if(video_control)
        {
            video_control->Release();
            video_control = nullptr;
        }
    }

    return;
}

void UvcMediaFoundationVideo::DeinitDevice()
{
    if(ksControl)
    {
        ksControl->Release();
        ksControl = nullptr;
    }

    ksControlNodeId = KS_CONTROL_NODE_ID_INVALID;

    if(baseFilter)
    {
        baseFilter->Release();
        baseFilter = nullptr;
    }

    if(sourceReader)
    {
        sourceReader->Release();
        sourceReader = nullptr;
    }

    if(asyncSourceReader)
    {
        asyncSourceReader->Release();
        asyncSourceReader = nullptr;
    }

    if(camera_control)
    {
        camera_control->Release();
        camera_control = nullptr;
    }

    if(video_control)
    {
        video_control->Release();
        video_control = nullptr;
    }

    if(mediaSource)
    {
        mediaSource->Shutdown();
        mediaSource->Release();
        mediaSource = nullptr;
    }

    streams.clear();
}

bool UvcMediaFoundationVideo::DeviceMatches(const std::wstring& symLink, int vendorId, int productId, std::wstring& instanceId)
{
    // Example symlink:
    // \\?\usb#vid_05a9&pid_0581&mi_00#6&2ff327a4&2&0000#{e5323777-f976-4f5b-9b55-b94699c46e44}\global
    //         ^^^^^^^^ ^^^^^^^^       ^^^^^^^^^^^^^^^^^
    std::wstring symLinkString(symLink);
    if(vendorId != 0 && !SymLinkIDMatches(symLinkString, L"vid_", vendorId))
    {
        return false;
    }

    if(productId != 0 && !SymLinkIDMatches(symLinkString, L"pid_", productId))
    {
        return false;
    }

    std::wstring instanceSearch = L"#" + instanceId + L"#";
    if(std::string::npos == symLinkString.find(instanceSearch))
    {
        return false;
    }

    return true;
}

bool UvcMediaFoundationVideo::SymLinkIDMatches(const std::wstring& symLink, const wchar_t* idStr, int id)
{
    // Find the ID prefix
    size_t idOffset = symLink.find(idStr);
    if(idOffset == std::wstring::npos)
    {
        // Unable to find the prefix
        return false;
    }

    // Parse the ID as a hexadecimal number
    int found = std::wcstol(&symLink[idOffset + std::wcslen(idStr)], nullptr, 16);
    return id == found;
}

PANGOLIN_REGISTER_FACTORY(UvcMediaFoundationVideo)
{
    struct UvcMediaFoundationVideoFactory final : public TypedFactoryInterface<VideoInterface>
    {
        std::map<std::string,Precedence> Schemes() const override
        {
            return {{"uvc",10}};
        }
        const char* Description() const override
        {
            return "Use Windows Media Foundation to open UVC USB device.";
        }
        ParamSet Params() const override
        {
            return {{
                {"size","640x480","Image dimension"},
                {"fps","0","Frames per second (0:unspecified)"},
                {"period","0","Specify frame period in microseconds (0:unspecified)"},
                {"num","0","Open the nth device (no need for vid and pid)"},
            }};
        }
        std::unique_ptr<VideoInterface> Open(const Uri& uri) override
        {
            int vendorId = 0;
            int productId = 0;

            const std::string instanceId = uri.url.substr(uri.url.rfind("\\") + 1);
            std::istringstream(uri.url.substr(uri.url.find("vid_", 0) + 4, 4)) >> std::hex >> vendorId;
            std::istringstream(uri.url.substr(uri.url.find("pid_", 0) + 4, 4)) >> std::hex >> productId;
            const ImageDim dim = uri.Get<ImageDim>("size", ImageDim(640, 480));
            unsigned int fps = uri.Get<unsigned int>("fps", 0);  // 0 means unspecified
            if (fps == 0 && uri.Contains("period")) {
              uint32_t period_us = uri.Get<uint32_t>("period", 0);
              fps = 1000000 / period_us;
            }

            return std::unique_ptr<VideoInterface>(new UvcMediaFoundationVideo(vendorId, productId, instanceId, dim.x, dim.y, fps));
        }
    };

    return FactoryRegistry::I()->RegisterFactory<VideoInterface>(std::make_shared<UvcMediaFoundationVideoFactory>());
}
}

