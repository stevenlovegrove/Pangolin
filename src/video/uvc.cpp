#include <pangolin/video/uvc.h>

namespace pangolin
{

UvcVideo::UvcVideo()
    : ctx_(NULL),
      dev_(NULL),
      devh_(NULL),
      frame_(NULL)
{
    uvc_init(&ctx_, NULL);
    if(!ctx_) {
        throw VideoException("Unable to open UVC Context");
    }
    
    InitDevice(0x03e7,0x1811,NULL, 640*2, 480, 45);
    Start();
}

UvcVideo::~UvcVideo()
{
    DeinitDevice();
    
//    if (ctx_) {
//        // Work out how to kill this properly
//        uvc_exit(ctx_);
//        ctx_ = 0;
//    }   
}

void UvcVideo::InitDevice(int vid, int pid, const char* sn, int width, int height, int fps)
{    
    uvc_error_t find_err = uvc_find_device(ctx_, &dev_, vid, pid, sn );
    if (find_err != UVC_SUCCESS) {
        uvc_perror(find_err, "uvc_find_device");
        throw VideoException("Unable to open UVC Device");
    }
    if(!dev_) {
        throw VideoException("Unable to open UVC Device - no pointer returned.");
    }
    
    uvc_error_t open_err = uvc_open(dev_, &devh_);
    if (open_err != UVC_SUCCESS) {
        uvc_perror(open_err, "uvc_open");
        uvc_unref_device(dev_);
        throw VideoException("Unable to open device");
    }
    
    uvc_error_t mode_err = uvc_get_stream_ctrl_format_size(
                devh_, &ctrl_,
                UVC_COLOR_FORMAT_GRAY8,
                width, height,
                fps);
            
    if (mode_err != UVC_SUCCESS) {
        uvc_perror(mode_err, "uvc_get_stream_ctrl_format_size");
        uvc_close(devh_);
        uvc_unref_device(dev_);
        throw VideoException("Unable to device mode.");
    }
    
    const VideoPixelFormat pfmt = VideoFormatFromString("GRAY8");
    const StreamInfo stream_info(pfmt, width, height, (width*pfmt.bpp)/8, 0);
    streams.push_back(stream_info);
}

void UvcVideo::DeinitDevice()
{
    Stop();
    
    if (frame_) {
        uvc_free_frame(frame_);
        frame_ = 0;
    }    
}

void UvcVideo::Start()
{
    uvc_error_t stream_err = uvc_start_iso_streaming(devh_, &ctrl_, NULL, this);
    
    if (stream_err != UVC_SUCCESS) {
        uvc_perror(stream_err, "uvc_start_iso_streaming");
        uvc_close(devh_);
        uvc_unref_device(dev_);
        throw VideoException("Unable to start iso streaming.");
    }
    
    if (frame_) {
        uvc_free_frame(frame_);
    }
    
    size_bytes = ctrl_.dwMaxVideoFrameSize;
    frame_ = uvc_allocate_frame(size_bytes);
    if(!frame_) {
        throw VideoException("Unable to allocate frame.");
    }
}

void UvcVideo::Stop()
{
    if(devh_) {
        uvc_stop_streaming(devh_);
    }
}

size_t UvcVideo::SizeBytes() const
{
    return size_bytes;
}

const std::vector<StreamInfo>& UvcVideo::Streams() const
{
    return streams;
}

bool UvcVideo::GrabNext( unsigned char* image, bool wait )
{
    uvc_frame_t* frame = NULL;
    uvc_error_t err = uvc_get_frame(devh_, &frame, 0);
    
    if(err!= UVC_SUCCESS) {
        uvc_perror(err, "uvc_get_frame");
        return false;
    }else{
        if(frame) {
            memcpy(image, frame->data, frame->data_bytes );
            return true;
        }else{
            std::cerr << "No data..." << std::endl;
            return false;
        }
    }
}

bool UvcVideo::GrabNewest( unsigned char* image, bool wait )
{
    return GrabNext(image, wait);
}

}
