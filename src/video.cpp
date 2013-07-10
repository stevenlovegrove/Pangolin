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

#include <pangolin/video.h>

#ifdef HAVE_DC1394
#include <pangolin/video/firewire.h>
#endif

#ifdef HAVE_V4L
#include <pangolin/video/v4l.h>
#endif

#ifdef HAVE_FFMPEG
#include <pangolin/video/ffmpeg.h>
#endif

#ifdef HAVE_OPENNI
#include <pangolin/video/openni.h>
#endif

#ifdef HAVE_UVC
#include <pangolin/video/uvc.h>
#endif

#include <pangolin/video/test.h>
#include <pangolin/video/pvn_video.h>
#include <pangolin/video_splitter.h>

#include <boost/algorithm/string.hpp>

namespace pangolin
{

std::istream& operator>> (std::istream &is, ImageDim &dim)
{
    is >> dim.x; is.get(); is >> dim.y;
    return is;
}

std::istream& operator>> (std::istream &is, ImageRoi &roi)
{
    is >> roi.x; is.get(); is >> roi.y; is.get();
    is >> roi.w; is.get(); is >> roi.h;
    return is;
}

#ifdef HAVE_DC1394

dc1394video_mode_t get_firewire_format7_mode(const std::string fmt)
{
    const std::string FMT7_prefix = "FORMAT7_";
    
    if( boost::algorithm::starts_with(fmt, FMT7_prefix) )
    {
        int fmt7_mode = 0;
        std::istringstream iss( fmt.substr(FMT7_prefix.size()) );
        iss >> fmt7_mode;
        if( !iss.fail() ) {
            return (dc1394video_mode_t)(DC1394_VIDEO_MODE_FORMAT7_0 + fmt7_mode);
        }
    }
    
    throw VideoException("Unknown video mode");
}

dc1394video_mode_t get_firewire_mode(unsigned width, unsigned height, const std::string fmt)
{
    for( dc1394video_mode_t video_mode=DC1394_VIDEO_MODE_MIN; video_mode<DC1394_VIDEO_MODE_MAX; video_mode = (dc1394video_mode_t)(video_mode +1) )
    {
        try {
            unsigned w,h;
            std::string format;
            Dc1394ModeDetails(video_mode,w,h,format);
            
            if( w == width && h==height && !fmt.compare(format) )
                return video_mode;
        } catch (VideoException e) {}
    }
    
    throw VideoException("Unknown video mode");
}

dc1394framerate_t get_firewire_framerate(float framerate)
{
    if(framerate==1.875)     return DC1394_FRAMERATE_1_875;
    else if(framerate==3.75) return DC1394_FRAMERATE_3_75;
    else if(framerate==7.5)  return DC1394_FRAMERATE_7_5;
    else if(framerate==15)   return DC1394_FRAMERATE_15;
    else if(framerate==30)   return DC1394_FRAMERATE_30;
    else if(framerate==60)   return DC1394_FRAMERATE_60;
    else if(framerate==120)  return DC1394_FRAMERATE_120;
    else if(framerate==240)  return DC1394_FRAMERATE_240;
    else throw VideoException("Invalid framerate");
}

#endif

#ifdef HAVE_OPENNI

OpenNiSensorType openni_sensor(std::string str)
{
    if( !str.compare("rgb") ) {
        return OpenNiRgb;
    }else if( !str.compare("ir") ) {
        return OpenNiIr;
    }else if( !str.compare("depth") ) {
        return OpenNiDepth;
    }else if( !str.compare("reg_depth") ) {
        return OpenNiDepthRegistered;
    }else if( !str.compare("ir8") ) {
        return OpenNiIr8bit;
    }else if( !str.compare("ir+") ) {
        return OpenNiIrProj;
    }else if( !str.compare("ir8+") ) {
        return OpenNiIr8bitProj;
    }else{
        throw pangolin::VideoException("Unknown OpenNi sensor", str );
    }
}

#endif // HAVE_OPENNI

VideoInterface* OpenVideo(std::string str_uri)
{
    VideoInterface* video = 0;
    
    Uri uri = ParseUri(str_uri);
    
    if(!uri.scheme.compare("test") )
    {
        const ImageDim dim = uri.Get<ImageDim>("size", ImageDim(640,480));
        const int n = uri.Get<int>("n", 1);
        std::string fmt  = uri.Get<std::string>("fmt","RGB24");        
        video = new TestVideo(dim.x,dim.y,n,fmt);
    }else
    if(!uri.scheme.compare("file") && boost::algorithm::ends_with(uri.url,"pvn") )
    {
        const bool realtime = uri.Contains("realtime");
        video = new PvnVideo(uri.url.c_str(), realtime);
    }else
    if(!uri.scheme.compare("split"))
    {
        std::vector<ImageRoi> rois;

        VideoInterface* subvid = OpenVideo(uri.url);
        const int subw = subvid->Streams()[0].Width();
        const int subh = subvid->Streams()[0].Height();
        const ImageRoi default_roi(0,0, subw, subh );
        
        while(true)
        {
            std::stringstream ss;
            ss << "roi" << (rois.size() + 1);
            const std::string key = ss.str();
            
            if(!uri.Contains(key)) {
                break;
            }
            
            rois.push_back( uri.Get<ImageRoi>(key, default_roi));
        }
        
        if(rois.size() == 0) {
            if(subw > subh) {
                // split horizontally
                rois.push_back( ImageRoi(0,0, subw/2, subh ) );
                rois.push_back( ImageRoi(subw/2,0, subw/2, subh ) );                
            }else{
                // split horizontally
                rois.push_back( ImageRoi(0,0, subw, subh/2 ) );
                rois.push_back( ImageRoi(0,subh/2, subw, subh/2 ) );                
            }
        }
        
        video = new VideoSplitter(subvid,rois);       
    }else
#ifdef HAVE_FFMPEG
    if(!uri.scheme.compare("ffmpeg") || !uri.scheme.compare("file") || !uri.scheme.compare("files") ){
        std::string outfmt = uri.Get<std::string>("fmt","RGB24");
        boost::to_upper(outfmt);
        const int video_stream = uri.Get<int>("stream",-1);
        video = new FfmpegVideo(uri.url.c_str(), outfmt, "", false, video_stream);
    }else if( !uri.scheme.compare("mjpeg")) {
        video = new FfmpegVideo(uri.url.c_str(),"RGB24", "MJPEG" );
    }else if( !uri.scheme.compare("convert") ) {
        std::string outfmt = uri.Get<std::string>("fmt","RGB24");
        boost::to_upper(outfmt);
        VideoInterface* subvid = OpenVideo(uri.url);
        video = new FfmpegConverter(subvid,outfmt,FFMPEG_POINT);
    }else
#endif //HAVE_FFMPEG
#ifdef HAVE_V4L
    if(!uri.scheme.compare("v4l")) {
        const std::string smethod = uri.Get<std::string>("method","mmap");
        
        io_method method = IO_METHOD_MMAP;
        
        if(smethod == "read" ) {
            method = IO_METHOD_READ;
        }else if(smethod == "mmap" ) {
            method = IO_METHOD_MMAP;
        }else if(smethod == "userptr" ) {
            method = IO_METHOD_USERPTR;
        }            
        
        video = new V4lVideo(uri.url.c_str(), method);
    }else
#endif // HAVE_V4L
#ifdef HAVE_DC1394
    if(!uri.scheme.compare("firewire") || !uri.scheme.compare("dc1394") ) {
        std::string desired_format = uri.Get<std::string>("fmt","RGB24");
        boost::to_upper(desired_format);
        const ImageDim desired_dim = uri.Get<ImageDim>("size", ImageDim(640,480));
        const ImageDim desired_xy  = uri.Get<ImageDim>("pos", ImageDim(0,0));
        const int desired_dma = uri.Get<int>("dma", 10);
        const int desired_iso = uri.Get<int>("iso", 400);
        const float desired_fps = uri.Get<float>("fps", 30);
                
        Guid guid = 0;
        unsigned deviceid = 0;
        dc1394framerate_t framerate = get_firewire_framerate(desired_fps);
        dc1394speed_t iso_speed = (dc1394speed_t)(log(desired_iso/100) / log(2));
        int dma_buffers = desired_dma;
        
        if( boost::algorithm::starts_with(desired_format, "FORMAT7") )
        {
            dc1394video_mode_t video_mode = get_firewire_format7_mode(desired_format);
            if( guid.guid == 0 ) {
                video = new FirewireVideo(deviceid,video_mode,FirewireVideo::MAX_FR, desired_dim.x, desired_dim.y, desired_xy.x, desired_xy.y, iso_speed, dma_buffers,true);
            }else{
                video = new FirewireVideo(guid,video_mode,FirewireVideo::MAX_FR, desired_dim.x, desired_dim.y, desired_xy.x, desired_xy.y, iso_speed, dma_buffers,true);
            }
        }else{
            dc1394video_mode_t video_mode = get_firewire_mode(desired_dim.x, desired_dim.y,desired_format);
            if( guid.guid == 0 ) {
                video = new FirewireVideo(deviceid,video_mode,framerate,iso_speed,dma_buffers);
            }else{
                video = new FirewireVideo(guid,video_mode,framerate,iso_speed,dma_buffers);
            }
        }
    }else
#endif //HAVE_DC1394
#ifdef HAVE_OPENNI
    if(!uri.scheme.compare("openni") || !uri.scheme.compare("kinect"))
    {
        OpenNiSensorType img1 = OpenNiRgb;
        OpenNiSensorType img2 = OpenNiUnassigned;
        
        if(uri.params.find("img1")!=uri.params.end()){
            img1 = openni_sensor(uri.params["img1"]);
        }
        
        if(uri.params.find("img2")!=uri.params.end()){
            img2 = openni_sensor(uri.params["img2"]);
        }
        
        video = new OpenNiVideo(img1,img2);
    }else
#endif
#ifdef HAVE_UVC
    if(!uri.scheme.compare("uvc")) {
        video = new UvcVideo();
    }else
#endif
    {
        throw VideoException("Unable to open video URI");
    }
    
    return video;
}

VideoInput::VideoInput()
    : uri(""), video(0)
{
}

VideoInput::VideoInput(const std::string& uri)
    : video(0)
{
    Open(uri);
}

VideoInput::~VideoInput()
{
    if(video) delete video;
}

void VideoInput::Open(const std::string& uri)
{
    this->uri = uri;
    
    if(video) {
        delete video;
        video = 0;
    }
    
    // Create video device
    video = OpenVideo(uri);
}

void VideoInput::Reset()
{
    Open(uri);
}

size_t VideoInput::SizeBytes() const
{
    if( !video ) throw VideoException("No video source open");
    return video->SizeBytes();
}

const std::vector<StreamInfo>& VideoInput::Streams() const
{
    if( !video ) throw VideoException("No video source open");
    return video->Streams();
}

unsigned VideoInput::Width() const
{
    if( !video ) throw VideoException("No video source open");
    return video->Streams()[0].Width();
}

unsigned VideoInput::Height() const
{
    if( !video ) throw VideoException("No video source open");
    return video->Streams()[0].Height();
}

VideoPixelFormat VideoInput::PixFormat() const
{
    if( !video ) throw VideoException("No video source open");
    return Streams()[0].PixFormat();
}

void VideoInput::Start()
{
    if( !video ) throw VideoException("No video source open");
    video->Start();
}

void VideoInput::Stop()
{
    if( !video ) throw VideoException("No video source open");
    video->Stop();
}

bool VideoInput::GrabNext( unsigned char* image, bool wait )
{
    if( !video ) throw VideoException("No video source open");
    return video->GrabNext(image,wait);
}

bool VideoInput::GrabNewest( unsigned char* image, bool wait )
{
    if( !video ) throw VideoException("No video source open");
    return video->GrabNext(image,wait);
}

bool VideoInput::Grab( unsigned char* buffer, std::vector<Image<unsigned char> >& images, bool wait, bool newest)
{
    if( !video ) throw VideoException("No video source open");
    
    bool success;
    
    if(newest) {
        success = GrabNewest(buffer, wait);
    }else{
        success = GrabNext(buffer, wait);
    }
    
    if(success) {
        images.clear();
        for(size_t s=0; s < Streams().size(); ++s) {
            images.push_back(Streams()[s].StreamImage(buffer));
        }
    }
    
    return success;
}

}
