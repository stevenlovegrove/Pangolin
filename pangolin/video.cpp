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

#include "video.h"

#ifdef HAVE_DC1394
#include "video/firewire.h"
#endif

#ifdef HAVE_V4L
#include "video/v4l.h"
#endif

#ifdef HAVE_FFMPEG
#include "video/ffmpeg.h"
#endif

#ifdef HAVE_OPENNI
#include "video/openni.h"
#endif

#include "video/pvn_video.h"

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/foreach.hpp>
#define foreach BOOST_FOREACH

using namespace std;
using namespace boost;

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
                    if( args.size() == 2 )
                    {
                        boost::trim(args[0]);
                        boost::trim(args[1]);
                        uri.params[args[0]] = args[1];
                    }
                }
            }else{
                throw VideoException("Bad video URI");
            }
        }

        uri.url = str_uri.substr(nurl+2);

//        // Find parameter delimiter
//        size_t nq = str_uri.find_first_of('?',nurl+2);
//        if(nq == string::npos)
//        {
//            uri.url = str_uri.substr(nurl+2);
//        }else{
//            string queries = str_uri.substr(nq+1);
//            uri.url = str_uri.substr(nurl+2,nq-(nurl+2));
//            vector<string> params;
//            split(params, queries, boost::is_any_of("&"));
//            foreach(string p, params)
//            {
//                vector<string> args;
//                split(args, p, boost::is_any_of("=") );
//                if( args.size() == 2 )
//                {
//                    uri.params[args[0]] = args[1];
//                }
//            }
//        }
    }

    return uri;
}

#ifdef HAVE_DC1394

dc1394video_mode_t get_firewire_format7_mode(const string fmt)
{
    const string FMT7_prefix = "FORMAT7_";

    if( algorithm::starts_with(fmt, FMT7_prefix) )
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
        unsigned w,h;
        string format;
        Dc1394ModeDetails(video_mode,w,h,format);

        if( w == width && h==height && !fmt.compare(format) )
            return video_mode;
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

    if(!uri.scheme.compare("file") && algorithm::ends_with(uri.url,"pvn") )
    {
        bool realtime = true;
        if(uri.params.find("realtime")!=uri.params.end()){
            std::istringstream iss(uri.params["realtime"]);
            iss >> realtime;
        }
        video = new PvnVideo(uri.url.c_str(), realtime);
    }else
#ifdef HAVE_FFMPEG
    if(!uri.scheme.compare("ffmpeg") || !uri.scheme.compare("file") || !uri.scheme.compare("files") ){
        string outfmt = "RGB24";
        if(uri.params.find("fmt")!=uri.params.end()){
            outfmt = uri.params["fmt"];
        }
        int video_stream = -1;
        if(uri.params.find("stream")!=uri.params.end()){
            std::istringstream iss(uri.params["stream"]);
            iss >> video_stream;
        }
        video = new FfmpegVideo(uri.url.c_str(), outfmt, "", false, video_stream);
    }else if( !uri.scheme.compare("mjpeg")) {
        video = new FfmpegVideo(uri.url.c_str(),"RGB24", "MJPEG" );
    }else if( !uri.scheme.compare("convert") ) {
        string outfmt = "RGB24";
        if(uri.params.find("fmt")!=uri.params.end()){
            outfmt = uri.params["fmt"];
        }
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
        // Default parameters
        int desired_x = 0;
        int desired_y = 0;
        int desired_width = 640;
        int desired_height = 480;
        int desired_dma = 10;
        int desired_iso = 400;
        float desired_fps = 30;
        string desired_format = "RGB24";

        // Parse parameters
        if(uri.params.find("fmt")!=uri.params.end()){
            desired_format = uri.params["fmt"];
            boost::to_upper(desired_format);
        }
        if(uri.params.find("size")!=uri.params.end()){
            std::istringstream iss(uri.params["size"]);
            iss >> desired_width;
            iss.get();
            iss >> desired_height;
        }
        if(uri.params.find("pos")!=uri.params.end()){
            std::istringstream iss(uri.params["pos"]);
            iss >> desired_x;
            iss.get();
            iss >> desired_y;
        }
        if(uri.params.find("dma")!=uri.params.end()){
            std::istringstream iss(uri.params["dma"]);
            iss >> desired_dma;
        }
        if(uri.params.find("iso")!=uri.params.end()){
            std::istringstream iss(uri.params["iso"]);
            iss >> desired_iso;
        }
        if(uri.params.find("fps")!=uri.params.end()){
            std::istringstream iss(uri.params["fps"]);
            iss >> desired_fps;
        }

        Guid guid = 0;
        unsigned deviceid = 0;
        dc1394framerate_t framerate = get_firewire_framerate(desired_fps);
        dc1394speed_t iso_speed = (dc1394speed_t)(log(desired_iso/100) / log(2));
        int dma_buffers = desired_dma;

        if( algorithm::starts_with(desired_format, "FORMAT7") )
        {
            dc1394video_mode_t video_mode = get_firewire_format7_mode(desired_format);
            if( guid.guid == 0 ) {
                video = new FirewireVideo(deviceid,video_mode,FirewireVideo::MAX_FR,desired_width, desired_height, desired_x, desired_y, iso_speed, dma_buffers,true);
            }else{
                video = new FirewireVideo(guid,video_mode,FirewireVideo::MAX_FR,desired_width, desired_height, desired_x, desired_y, iso_speed, dma_buffers,true);
            }
        }else{
            dc1394video_mode_t video_mode = get_firewire_mode(desired_width,desired_height,desired_format);
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
    video = OpenVideo(uri);
}

void VideoInput::Reset()
{
    Open(uri);
}

unsigned VideoInput::Width() const
{
    if( !video ) throw VideoException("No video source open");
    return video->Width();
}

unsigned VideoInput::Height() const
{
    if( !video ) throw VideoException("No video source open");
    return video->Height();
}

size_t VideoInput::SizeBytes() const
{
    if( !video ) throw VideoException("No video source open");
    return video->SizeBytes();
}

std::string VideoInput::PixFormat() const
{
    if( !video ) throw VideoException("No video source open");
    return video->PixFormat();
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

}
