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

#include <pangolin/video/drivers/uvc.h>

namespace pangolin
{

UvcVideo::UvcVideo(int vendor_id, int product_id, const char* sn, int device_id, int width, int height, int fps)
    : ctx_(NULL),
      dev_(NULL),
      devh_(NULL),
      frame_(NULL)
{
    uvc_init(&ctx_, NULL);
    if(!ctx_) {
        throw VideoException("Unable to open UVC Context");
    }
    
    InitDevice(vendor_id, product_id, sn, device_id, width, height, fps);
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

uvc_error_t UvcVideo::FindDevice(
    uvc_context_t *ctx, uvc_device_t **dev,
    int vid, int pid, const char *sn, int device_id) {
  uvc_error_t ret = UVC_SUCCESS;

  uvc_device_t **list;
  uvc_device_t *test_dev;

  ret = uvc_get_device_list(ctx, &list);

  if (ret != UVC_SUCCESS) {
    return ret;
  }

  int dev_idx = 0;
  int num_found = 0;
  bool found_dev = false;

  while (!found_dev && (test_dev = list[dev_idx++]) != NULL) {
    uvc_device_descriptor_t *desc;

    fprintf(stderr,"Here\n");

    if (uvc_get_device_descriptor(test_dev, &desc) != UVC_SUCCESS)
      continue;

    fprintf(stderr,"found: vid: %i, pid: %i\n", desc->idVendor, desc->idProduct );
    fflush(stderr);

    const bool matches = (!vid || desc->idVendor == vid)
            && (!pid || desc->idProduct == pid)
            && (!sn || (desc->serialNumber && !strcmp(desc->serialNumber, sn)));

    uvc_free_device_descriptor(desc);

    if (matches) {
        if(device_id == num_found) {
            found_dev = true;
            break;
        }
        num_found++;
    }
  }

  if (found_dev)
    uvc_ref_device(test_dev);

  uvc_free_device_list(list, 1);

  if (found_dev) {
    *dev = test_dev;
    return UVC_SUCCESS;
  } else {
    return UVC_ERROR_NO_DEVICE;
  }
}

void UvcVideo::InitDevice(int vid, int pid, const char* sn, int device_id, int width, int height, int fps)
{
    uvc_error_t find_err = FindDevice(ctx_, &dev_, vid, pid, sn, device_id );
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

    //uvc_print_diag(devh_, stderr);
    
    uvc_error_t mode_err = uvc_get_stream_ctrl_format_size(
                devh_, &ctrl_,
                UVC_FRAME_FORMAT_YUYV,
                width, height,
                fps);

    //uvc_print_stream_ctrl(&ctrl_, stderr);
            
    if (mode_err != UVC_SUCCESS) {
        uvc_perror(mode_err, "uvc_get_stream_ctrl_format_size");
        uvc_close(devh_);
        uvc_unref_device(dev_);
        throw VideoException("Unable to set device mode.");
    }

    uvc_error_t strm_err = uvc_stream_open_ctrl(devh_, &strm_, &ctrl_);
    if(strm_err != UVC_SUCCESS) {
        uvc_perror(strm_err, "uvc_stream_open_ctrl");
        uvc_close(devh_);
        uvc_unref_device(dev_);
        throw VideoException("Unable to open device stream.");
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
    uvc_error_t stream_err = uvc_stream_start(strm_, NULL, this, 0);
    
    if (stream_err != UVC_SUCCESS) {
        uvc_perror(stream_err, "uvc_stream_start");
        uvc_close(devh_);
        uvc_unref_device(dev_);
        throw VideoException("Unable to start streaming.");
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
    uvc_error_t err = uvc_stream_get_frame(strm_, &frame, 0);
    
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

int UvcVideo::GetCtrl(uint8_t unit, uint8_t ctrl, void *data, int len, enum uvc_req_code req_code)
{
    return uvc_get_ctrl(devh_, unit, ctrl, data, len, req_code);
}

int UvcVideo::SetCtrl(uint8_t unit, uint8_t ctrl, void *data, int len)
{
    return uvc_set_ctrl(devh_, unit, ctrl, data, len);
}


}
