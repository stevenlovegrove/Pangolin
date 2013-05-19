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

#include <pangolin/video/pvn_video.h>
#include <pangolin/video_splitter.h>

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/foreach.hpp>
#define foreach BOOST_FOREACH

using namespace std;

namespace pangolin
{

const VideoPixelFormat SupportedVideoPixelFormats[] =
{
    {"GRAY8", 1, {8}, 8, false},
    {"GRAY16LE", 1, {16}, 16, false},
    {"RGB24", 3, {8,8,8}, 24, false},
    {"BGR24", 3, {8,8,8}, 24, false},
    {"YUYV422", 3, {4,2,2}, 16, false},
    {"",0,{0,0,0,0},0,0}
};

VideoPixelFormat VideoFormatFromString(const std::string& format)
{
    for(int i=0; !SupportedVideoPixelFormats[i].format.empty(); ++i)
        if(!format.compare(SupportedVideoPixelFormats[i].format))
            return SupportedVideoPixelFormats[i];
    throw VideoException("Unknown Format",format);
}

ostream& operator<< (ostream &out, Uri &uri)
{
    out << "scheme: " << uri.scheme << endl;
    out << "url:    " << uri.url << endl;
    out << "params:" << endl;
    typedef pair<string,string> Param;
    foreach( Param p, uri.params)
    {
        cout << "\t" << p.first << " = " << p.second << endl;
    }
    
    return out;
}

istream& operator>> (istream &is, ImageDim &dim)
{
    is >> dim.x; is.get(); is >> dim.y;
    return is;
}

istream& operator>> (istream &is, ImageRoi &roi)
{
    is >> roi.x; is.get(); is >> roi.y; is.get();
    is >> roi.w; is.get(); is >> roi.h;
    return is;
}

VideoInput::VideoInput()
    : uri(""), video(0)
{
}

VideoInput::VideoInput(std::string uri)
    : video(0)
{
    Open(uri);
}

VideoInput::~VideoInput()
{
    if(video) delete video;
}

Uri ParseUri(string str_uri)
{
    Uri uri;
    
    // Find Scheme delimiter
    size_t ns = str_uri.find_first_of(':');
    if( ns != string::npos )
    {
        uri.scheme = str_uri.substr(0,ns);
    }else{
//        throw VideoException("Bad video URI","no device scheme specified");
        uri.scheme = "file";
        uri.url = str_uri;
        return uri;
    }
    
    // Find url delimiter
    size_t nurl = str_uri.find("//",ns+1);
    if(nurl != string::npos)
    {
        // If there is space between the delimiters, extract protocol arguments
        if( nurl-ns > 1)
        {
            if( str_uri[ns+1] == '[' && str_uri[nurl-1] == ']' )
            {
                string queries = str_uri.substr(ns+2, nurl-1 - (ns+2) );
                vector<string> params;
                split(params, queries, boost::is_any_of(","));
                foreach(string p, params)
                {
                    vector<string> args;
                    split(args, p, boost::is_any_of("=") );
                    std::string key = args[0];
                    std::string val = args.size() > 1 ? args[1] : "";
                    boost::trim(key);
                    boost::trim(val);
                    uri.params[key] = val;
                }
            }else{
                throw VideoException("Bad video URI");
            }
        }
        
        uri.url = str_uri.substr(nurl+2);
    }
    
    return uri;
}

#ifdef HAVE_DC1394

dc1394video_mode_t get_firewire_format7_mode(const string fmt)
{
    const string FMT7_prefix = "FORMAT7_";
    
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

dc1394video_mode_t get_firewire_mode(unsigned width, unsigned height, const string fmt)
{
    for( dc1394video_mode_t video_mode=DC1394_VIDEO_MODE_MIN; video_mode<DC1394_VIDEO_MODE_MAX; video_mode = (dc1394video_mode_t)(video_mode +1) )
    {
        try {
            unsigned w,h;
            string format;
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

VideoInterface* OpenVideo(std::string str_uri)
{
    VideoInterface* video = 0;
    
    Uri uri = ParseUri(str_uri);
    
    if(!uri.scheme.compare("file") && boost::algorithm::ends_with(uri.url,"pvn") )
    {
        const bool realtime = uri.Contains("realtime");
        video = new PvnVideo(uri.url.c_str(), realtime);
    }else
    if(!uri.scheme.compare("split"))
    {
        std::vector<ImageRoi> rois;

        VideoInterface* subvid = OpenVideo(uri.url);
        const ImageRoi default_roi(0,0, subvid->Streams()[0].Width(), subvid->Streams()[0].Height() );
        
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
        
        video = new VideoSplitter(subvid,rois);       
    }else
#ifdef HAVE_FFMPEG
    if(!uri.scheme.compare("ffmpeg") || !uri.scheme.compare("file") || !uri.scheme.compare("files") ){
        string outfmt = uri.Get<std::string>("fmt","RGB24");
        boost::to_upper(outfmt);
        const int video_stream = uri.Get<int>("stream",-1);
        video = new FfmpegVideo(uri.url.c_str(), outfmt, "", false, video_stream);
    }else if( !uri.scheme.compare("mjpeg")) {
        video = new FfmpegVideo(uri.url.c_str(),"RGB24", "MJPEG" );
    }else if( !uri.scheme.compare("convert") ) {
        string outfmt = uri.Get<std::string>("fmt","RGB24");
        boost::to_upper(outfmt);
        VideoInterface* subvid = OpenVideo(uri.url);
        video = new FfmpegConverter(subvid,outfmt,FFMPEG_POINT);
    }else
#endif //HAVE_FFMPEG
#ifdef HAVE_V4L
    if(!uri.scheme.compare("v4l")) {
        video = new V4lVideo(uri.url.c_str());
    }else
#endif // HAVE_V4L
#ifdef HAVE_DC1394
    if(!uri.scheme.compare("firewire") || !uri.scheme.compare("dc1394") ) {
        string desired_format = uri.Get<std::string>("fmt","RGB24");
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
            std::istringstream iss(uri.params["img1"]);
            
            if( boost::iequals(iss.str(),"rgb") ) {
                img1 = OpenNiRgb;
            }else if( boost::iequals(iss.str(),"ir") ) {
                img1 = OpenNiIr;
            }else if( boost::iequals(iss.str(),"depth") ) {
                img1 = OpenNiDepth;
            }
        }
        
        if(uri.params.find("img2")!=uri.params.end()){
            std::istringstream iss(uri.params["img2"]);
            
            if( boost::iequals(iss.str(),"rgb") ) {
                img2 = OpenNiRgb;
            }else if( boost::iequals(iss.str(),"ir") ) {
                img2 = OpenNiIr;
            }else if( boost::iequals(iss.str(),"depth") ) {
                img2 = OpenNiDepth;
            }
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

void VideoInput::Open(std::string uri)
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
