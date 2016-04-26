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

#include <pangolin/video/video.h>

#ifdef HAVE_DC1394
#include <pangolin/video/drivers/firewire.h>
#include <pangolin/video/drivers/firewire_deinterlace.h>
#endif

#ifdef HAVE_V4L
#include <pangolin/video/drivers/v4l.h>
#endif

#ifdef HAVE_FFMPEG
#include <pangolin/video/drivers/ffmpeg.h>
#endif

#ifdef HAVE_OPENNI
#include <pangolin/video/drivers/openni.h>
#endif

#ifdef HAVE_OPENNI2
#include <pangolin/video/drivers/openni2.h>
#endif

#ifdef HAVE_UVC
#include <pangolin/video/drivers/uvc.h>
#endif

#ifdef HAVE_DEPTHSENSE
#include <pangolin/video/drivers/depthsense.h>
#endif

#ifdef HAVE_TELICAM
#include <pangolin/video/drivers/teli.h>
#endif

#ifdef HAVE_PLEORA
#include <pangolin/video/drivers/pleora.h>
#endif

#include <pangolin/utils/file_utils.h>
#include <pangolin/utils/file_extension.h>
#include <pangolin/video/drivers/test.h>
#include <pangolin/video/drivers/images.h>
#include <pangolin/video/drivers/pvn_video.h>
#include <pangolin/video/drivers/pango_video.h>
#include <pangolin/video/drivers/video_splitter.h>
#include <pangolin/video/drivers/debayer.h>
#include <pangolin/video/drivers/shift.h>
#include <pangolin/video/drivers/mirror.h>
#include <pangolin/video/drivers/unpack.h>
#include <pangolin/video/drivers/join.h>
#include <pangolin/video/drivers/thread.h>

#include <map>

#ifdef _UNIX_
#include <pangolin/utils/posix/shared_memory_buffer.h>
#include <pangolin/video/drivers/shared_memory.h>
#endif

namespace pangolin
{

std::istream& operator>> (std::istream &is, ImageDim &dim)
{
    if(std::isdigit(is.peek()) ) {
        // Expect 640x480, 640*480, ...
        is >> dim.x; is.get(); is >> dim.y;
    }else{
        // Expect 'VGA', 'QVGA', etc
        std::string sdim;
        is >> sdim;
        ToUpper(sdim);

        if( !sdim.compare("QQVGA") ) {
            dim = ImageDim(160,120);
        }else if( !sdim.compare("HQVGA") ) {
            dim = ImageDim(240,160);
        }else if( !sdim.compare("QVGA") ) {
            dim = ImageDim(320,240);
        }else if( !sdim.compare("WQVGA") ) {
            dim = ImageDim(360,240);
        }else if( !sdim.compare("HVGA") ) {
            dim = ImageDim(480,320);
        }else if( !sdim.compare("VGA") ) {
            dim = ImageDim(640,480);
        }else if( !sdim.compare("WVGA") ) {
            dim = ImageDim(720,480);
        }else if( !sdim.compare("SVGA") ) {
            dim = ImageDim(800,600);
        }else if( !sdim.compare("DVGA") ) {
            dim = ImageDim(960,640);
        }else if( !sdim.compare("WSVGA") ) {
            dim = ImageDim(1024,600);
        }else{
            throw VideoException("Unrecognised image-size string.");
        }
    }
    return is;
}

std::istream& operator>> (std::istream &is, ImageRoi &roi)
{
    is >> roi.x; is.get(); is >> roi.y; is.get();
    is >> roi.w; is.get(); is >> roi.h;
    return is;
}

std::istream& operator>> (std::istream &is, MirrorOptions &mirror)
{
    std::string str_mirror;
    is >> str_mirror;
    std::transform(str_mirror.begin(), str_mirror.end(), str_mirror.begin(), toupper);

    if(!str_mirror.compare("NONE")) {
        mirror = MirrorOptionsNone;
    }else if(!str_mirror.compare("FLIPX")) {
        mirror = MirrorOptionsFlipX;
    }else if(!str_mirror.compare("FLIPY")) {
        mirror = MirrorOptionsFlipY;
    }else if(!str_mirror.compare("FLIPXY")) {
        mirror = MirrorOptionsFlipXY;
    }else{
        pango_print_warn("Unknown mirror option %s.", str_mirror.c_str());
        mirror = MirrorOptionsNone;
    }

    return is;
}

std::istream& operator>> (std::istream &is, VideoPixelFormat& fmt)
{
    std::string sfmt;
    is >> sfmt;
    fmt = VideoFormatFromString(sfmt);
    return is;
}

std::istream& operator>> (std::istream &is, Image<unsigned char>& img)
{
    size_t offset;
    is >> offset; is.get();
    img.ptr = (unsigned char*)0 + offset;
    is >> img.w; is.get();
    is >> img.h; is.get();
    is >> img.pitch;
    return is;
}

std::istream& operator>> (std::istream &is, StreamInfo &stream)
{
    VideoPixelFormat fmt;
    Image<unsigned char> img_offset;
    is >> img_offset; is.get();
    is >> fmt;
    stream = StreamInfo(fmt, img_offset);
    return is;
}

std::vector<std::string> SplitBrackets(const std::string src, char open = '{', char close = '}')
{
    std::vector<std::string> splits;

    int nesting = 0;
    int begin = -1;

    for(size_t i=0; i < src.length(); ++i) {
        if(src[i] == open) {
            if(nesting==0) {
                begin = (int)i;
            }
            nesting++;
        }else if(src[i] == close) {
            nesting--;
            if(nesting == 0) {
                // matching close bracket.
                int str_start = begin+1;
                splits.push_back( src.substr(str_start, i-str_start) );
            }
        }
    }

    return splits;
}

#ifdef HAVE_DC1394

dc1394video_mode_t get_firewire_format7_mode(const std::string fmt)
{
    const std::string FMT7_prefix = "FORMAT7_";
    
    if( StartsWith(fmt, FMT7_prefix) )
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

#if defined(HAVE_OPENNI) || defined(HAVE_OPENNI2)

OpenNiSensorType openni_sensor(const std::string& str)
{
    if( !str.compare("grey") || !str.compare("gray") ) {
        return OpenNiGrey;
    }else if( !str.compare("rgb") ) {
        return OpenNiRgb;
    }else if( !str.compare("ir") ) {
        return OpenNiIr;
    }else if( !str.compare("depth1mm") || !str.compare("depth") ) {
        return OpenNiDepth_1mm;
    }else if( !str.compare("depth100um") ) {
        return OpenNiDepth_100um;
    }else if( !str.compare("depth_reg") || !str.compare("reg_depth")) {
        return OpenNiDepth_1mm_Registered;
    }else if( !str.compare("ir8") ) {
        return OpenNiIr8bit;
    }else if( !str.compare("ir24") ) {
        return OpenNiIr24bit;
    }else if( !str.compare("ir+") ) {
        return OpenNiIrProj;
    }else if( !str.compare("ir8+") ) {
        return OpenNiIr8bitProj;
    }else if( str.empty() ) {
        return OpenNiUnassigned;
    }else{
        throw pangolin::VideoException("Unknown OpenNi sensor", str );
    }
}

// Find prefix character key
// Given arguments "depth!5:320x240@15", "!:@", would return map
// \0->"depth", !->"5", :->"320x240", @->"15"
std::map<char,std::string> GetTokenSplits(const std::string& str, const std::string& tokens)
{
    std::map<char,std::string> splits;

    char last_token = 0;
    size_t  last_start = 0;
    for(unsigned int i=0; i<str.size(); ++i) {
        size_t token_pos = tokens.find(str[i]);
        if(token_pos != std::string::npos) {
            splits[last_token] = str.substr(last_start, i-last_start);
            last_token = tokens[token_pos];
            last_start = i+1;
        }
    }

    if(last_start < str.size()) {
        splits[last_token] = str.substr(last_start);
    }

    return splits;
}

std::istream& operator>> (std::istream &is, OpenNiStreamMode& fmt)
{
    std::string str;
    is >> str;

    std::map<char,std::string> splits = GetTokenSplits(str, "!:@");

    if(splits.count(0)) {
        fmt.sensor_type = openni_sensor(splits[0]);
    }

    if(splits.count('@')) {
        fmt.fps = pangolin::Convert<int,std::string>::Do(splits['@']);
    }

    if(splits.count(':')) {
        fmt.dim = pangolin::Convert<ImageDim,std::string>::Do(splits[':']);
    }

    if(splits.count('!')) {
        fmt.device = pangolin::Convert<int,std::string>::Do(splits['!']);
    }

    return is;
}

#endif // defined(HAVE_OPENNI) || defined(HAVE_OPENNI2)

#if defined(HAVE_DEPTHSENSE)

DepthSenseSensorType depthsense_sensor(const std::string& str)
{
    if (!str.compare("rgb")) {
        return DepthSenseRgb;
    }
    else if (!str.compare("depth")) {
        return DepthSenseDepth;
    }
    else if (str.empty()) {
        return DepthSenseUnassigned;
    }
    else{
        throw pangolin::VideoException("Unknown DepthSense sensor", str);
    }
}

#endif // defined(HAVE_DEPTHSENSE)

static std::map<std::string, VideoInterfaceFactory> s_RegisteredUriSchemes;

VideoInterface* OpenVideo(const std::string& str_uri)
{
    return OpenVideo( ParseUri(str_uri) );
}

VideoInterface* OpenVideo(const Uri& uri)
{
    VideoInterface* video = 0;

    // Allow a client to override internal implementations by checking
    // registered schemes first.
    std::map<std::string, VideoInterfaceFactory>::const_iterator it =
        std::begin(s_RegisteredUriSchemes);
    while (it != std::end(s_RegisteredUriSchemes)) {
        if (!uri.scheme.compare(it->first)) {
            return it->second(uri);
        }
    }

    if(!uri.scheme.compare("test") )
    {
        const ImageDim dim = uri.Get<ImageDim>("size", ImageDim(640,480));
        const int n = uri.Get<int>("n", 1);
        std::string fmt  = uri.Get<std::string>("fmt","RGB24");        
        video = new TestVideo(dim.x,dim.y,n,fmt);
    }else
    // '%' printf specifier used with ffmpeg
    if((!uri.scheme.compare("files") && uri.url.find('%') == std::string::npos) ||
        !uri.scheme.compare("file") || !uri.scheme.compare("pango") ||
        !uri.scheme.compare("pvn") )
    {
        const bool raw = uri.Contains("fmt");
        const std::string path = PathExpand(uri.url);
        ImageFileType ft = ImageFileTypeUnknown;

        if( !uri.scheme.compare("pango") ) {
            // User has forced pango type with uri
            ft = ImageFileTypePango;
        }else if(!uri.scheme.compare("pvn")) {
            // User has forced pvn type with uri
            ft = ImageFileTypePvn;
        }else{
            // Infer type from filecontents / filename.
            ft = FileType(uri.url);
        }

        if( ft == ImageFileTypePvn ) {
            const bool realtime = uri.Contains("realtime");
            video = new PvnVideo(path.c_str(), realtime);
        }else if(ft == ImageFileTypePango ) {
            const bool realtime = uri.Contains("realtime");
            video = new PangoVideo(path.c_str(), realtime);
        }else{
            if(raw) {
                const std::string sfmt = uri.Get<std::string>("fmt", "GRAY8");
                const VideoPixelFormat fmt = VideoFormatFromString(sfmt);
                const ImageDim dim = uri.Get<ImageDim>("size", ImageDim(640,480));
                video = new ImagesVideo(path, fmt, dim.x, dim.y);
            }else{
                video = new ImagesVideo(path);
            }
        }
    }else
    if(!uri.scheme.compare("debayer"))
    {
        VideoInterface* subvid = OpenVideo(uri.url);
        std::string tile_string = uri.Get<std::string>("tile","rggb");
        std::string method_string = uri.Get<std::string>("method","downsample");
        color_filter_t tile = DebayerVideo::ColorFilterFromString(tile_string);
        bayer_method_t method = DebayerVideo::BayerMethodFromString(method_string);
        video = new DebayerVideo(subvid, tile, method);
    }else
    if(!uri.scheme.compare("shift"))
    {
        const int shift_right = uri.Get<int>("shift", 0);
        const int mask = uri.Get<int>("mask",  0xffff);

        VideoInterface* subvid = OpenVideo(uri.url);
        video = new ShiftVideo(subvid, VideoFormatFromString("GRAY8"), shift_right, mask);
    }else
    if(!uri.scheme.compare("mirror"))
    {
        VideoInterface* subvid = OpenVideo(uri.url);

        std::vector<MirrorOptions> flips;

        for(size_t i=0; i < subvid->Streams().size(); ++i){
            std::stringstream ss;
            ss << "stream" << i;
            const std::string key = ss.str();
            flips.push_back(uri.Get<MirrorOptions>(key, MirrorOptionsFlipX) );
        }

        video = new MirrorVideo(subvid, flips);
    }else
    if(!uri.scheme.compare("unpack"))
    {
        VideoInterface* subvid = OpenVideo(uri.url);
        const bool make_float = uri.Contains("float");
        if(make_float) {
            video = new UnpackVideo(subvid, VideoFormatFromString("GRAY32F"));
        }else{
            video = new UnpackVideo(subvid, VideoFormatFromString("GRAY16LE"));
        }
    }else
    if(!uri.scheme.compare("join"))
    {
        std::vector<std::string> uris = SplitBrackets(uri.url);
        std::vector<VideoInterface*> src;

        if(uris.size() == 0) {
            throw VideoException("No VideoSources found in join URL.", "Specify videos to join with curly braces, e.g. join://{test://}{test://}");
        }

        for(size_t i=0; i<uris.size(); ++i) {
            src.push_back( OpenVideo(uris[i]) );
        }

        video = new VideoJoiner(src);

        const unsigned long sync_tol_us = uri.Get<unsigned long>("sync_tolerance_us", 0);
        const bool sync_continuously = uri.Get<bool>("sync_continuously", false);
        if(sync_tol_us>0) {
            if(!static_cast<VideoJoiner*>(video)->Sync(sync_tol_us, sync_continuously)) {
                pango_print_error("Error not all streams in join support sync_tolerance_us option.\n");
            }
        }
    }else
    if(!uri.scheme.compare("split"))
    {
        std::vector<StreamInfo> streams;

        VideoInterface* subvid = OpenVideo(uri.url);
        if(subvid->Streams().size() != 1)
            throw VideoException("VideoSplitter input must have exactly one stream");

        const size_t subw = subvid->Streams()[0].Width();
        const size_t subh = subvid->Streams()[0].Height();
        const ImageRoi default_roi(0,0, subw, subh );
        const StreamInfo& stmin = subvid->Streams()[0];

        while(true) {
            std::stringstream ss;
            ss << "roi" << (streams.size() + 1);
            const std::string key = ss.str();

            if(uri.Contains(key)) {
                const ImageRoi& roi = uri.Get<ImageRoi>(key, default_roi);
                const size_t start1 = roi.y * stmin.Pitch() + stmin.PixFormat().bpp * roi.x / 8;
                streams.push_back( StreamInfo( stmin.PixFormat(), roi.w, roi.h, stmin.Pitch(), (unsigned char*)0 + start1 ) );
            }else{
                std::stringstream ss;
                ss << "mem" << (streams.size() + 1);
                const std::string key = ss.str();
                if(uri.Contains(key)) {
                    const StreamInfo& info = uri.Get<StreamInfo>(key, stmin);
                    streams.push_back(info);
                }else{
                    break;
                }
            }
        }

        // Default split if no arguments
        if(streams.size() == 0) {
            ImageRoi roi1, roi2;

            if(subw > subh) {
                // split horizontally
                roi1 = ImageRoi(0,0, subw/2, subh );
                roi2 = ImageRoi(subw/2,0, subw/2, subh );
            }else{
                // split vertically
                roi1 = ImageRoi(0,0, subw, subh/2 );
                roi2 = ImageRoi(0,subh/2, subw, subh/2 );
            }

            const size_t start1 = roi1.y * stmin.Pitch() + stmin.PixFormat().bpp * roi1.x / 8;
            const size_t start2 = roi2.y * stmin.Pitch() + stmin.PixFormat().bpp * roi2.x / 8;
            streams.push_back( StreamInfo( stmin.PixFormat(), roi1.w, roi1.h, stmin.Pitch(), (unsigned char*)0 + start1 ) );
            streams.push_back( StreamInfo( stmin.PixFormat(), roi2.w, roi2.h, stmin.Pitch(), (unsigned char*)0 + start2 ) );
        }
        
        video = new VideoSplitter(subvid,streams);
    }else
    if(!uri.scheme.compare("thread"))
    {
        VideoInterface* subvid = OpenVideo(uri.url);
        const int num_buffers = uri.Get<int>("num_buffers", 30);
        video = new ThreadVideo(subvid, num_buffers);
    }else
#ifdef HAVE_FFMPEG
    if(!uri.scheme.compare("ffmpeg") || !uri.scheme.compare("file") || !uri.scheme.compare("files") ){
        std::string outfmt = uri.Get<std::string>("fmt","RGB24");
        ToUpper(outfmt);
        const int video_stream = uri.Get<int>("stream",-1);
        video = new FfmpegVideo(uri.url.c_str(), outfmt, "", false, video_stream);
    }else if( !uri.scheme.compare("mjpeg")) {
        video = new FfmpegVideo(uri.url.c_str(),"RGB24", "MJPEG" );
    }else if( !uri.scheme.compare("convert") ) {
        std::string outfmt = uri.Get<std::string>("fmt","RGB24");
        ToUpper(outfmt);
        VideoInterface* subvid = OpenVideo(uri.url);
        video = new FfmpegConverter(subvid,outfmt,FFMPEG_POINT);
    }else
#endif //HAVE_FFMPEG
#ifdef HAVE_V4L
    if(!uri.scheme.compare("v4l")) {
        const std::string smethod = uri.Get<std::string>("method","mmap");
        const ImageDim desired_dim = uri.Get<ImageDim>("size", ImageDim(0,0));
        
        io_method method = IO_METHOD_MMAP;
        
        if(smethod == "read" ) {
            method = IO_METHOD_READ;
        }else if(smethod == "mmap" ) {
            method = IO_METHOD_MMAP;
        }else if(smethod == "userptr" ) {
            method = IO_METHOD_USERPTR;
        }            
        
        video = new V4lVideo(uri.url.c_str(), method, desired_dim.x, desired_dim.y );
    }else
#endif // HAVE_V4L
#ifdef HAVE_DC1394
    if(!uri.scheme.compare("firewire") || !uri.scheme.compare("dc1394") ) {
        std::string desired_format = uri.Get<std::string>("fmt","RGB24");
        ToUpper(desired_format);
        const ImageDim desired_dim = uri.Get<ImageDim>("size", ImageDim(640,480));
        const ImageDim desired_xy  = uri.Get<ImageDim>("pos", ImageDim(0,0));
        const int desired_dma = uri.Get<int>("dma", 10);
        const int desired_iso = uri.Get<int>("iso", 400);
        const float desired_fps = uri.Get<float>("fps", 30);
        const bool deinterlace = uri.Get<bool>("deinterlace", 0);

        Guid guid = 0;
        unsigned deviceid = 0;
        dc1394framerate_t framerate = get_firewire_framerate(desired_fps);
        dc1394speed_t iso_speed = (dc1394speed_t)(log(desired_iso/100) / log(2));
        int dma_buffers = desired_dma;
        
        if( StartsWith(desired_format, "FORMAT7") )
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

        if(deinterlace) {
            video = new FirewireDeinterlace(video);
        }
    }else
#endif //HAVE_DC1394
#ifdef HAVE_OPENNI
    if(!uri.scheme.compare("openni") || !uri.scheme.compare("kinect"))
    {
        const ImageDim dim = uri.Get<ImageDim>("size", ImageDim(640,480));
        const unsigned int fps = uri.Get<unsigned int>("fps", 30);
        const bool autoexposure = uri.Get<bool>("autoexposure", true);

        OpenNiSensorType img1 = OpenNiRgb;
        OpenNiSensorType img2 = OpenNiUnassigned;
        
        if(uri.params.find("img1")!=uri.params.end()){
            img1 = openni_sensor(uri.Get<std::string>("img1", "depth"));
        }
        
        if(uri.params.find("img2")!=uri.params.end()){
            img2 = openni_sensor(uri.Get<std::string>("img2","rgb"));
        }

        OpenNiVideo* oniv = new OpenNiVideo(img1, img2, dim, fps);
        oniv->SetAutoExposure(autoexposure);
        video = oniv;
    }else
#endif
#ifdef HAVE_OPENNI2
    if(!uri.scheme.compare("openni2") )
    {
        const bool realtime = uri.Contains("realtime");
        const ImageDim default_dim = uri.Get<ImageDim>("size", ImageDim(640,480));
        const unsigned int default_fps = uri.Get<unsigned int>("fps", 30);

        std::vector<OpenNiStreamMode> stream_modes;

        int num_streams = 0;
        std::string simg= "img1";
        while(uri.Contains(simg)) {
            OpenNiStreamMode stream = uri.Get<OpenNiStreamMode>(simg, OpenNiStreamMode(OpenNiRgb,default_dim,default_fps,0));
            stream_modes.push_back(stream);
            ++num_streams;
            simg = "img" + ToString(num_streams+1);
        }

        OpenNiVideo2* nivid;
        if(!uri.url.empty()) {
            nivid = new OpenNiVideo2(pangolin::PathExpand(uri.url));
        }else if(stream_modes.size()) {
            nivid = new OpenNiVideo2(stream_modes);
        }else{
            nivid = new OpenNiVideo2(default_dim, default_fps);
        }

        nivid->SetDepthCloseRange( uri.Get<bool>("closerange",false) );
        nivid->SetDepthHoleFilter( uri.Get<bool>("holefilter",false) );
        nivid->SetDepthColorSyncEnabled( uri.Get<bool>("coloursync",false) );
        nivid->SetPlaybackSpeed(realtime ? 1.0f : -1.0f);
        nivid->SetAutoExposure(true);
        nivid->SetAutoWhiteBalance(true);
        nivid->SetMirroring(false);

        nivid->UpdateProperties();

        video = nivid;
    }else
#endif
#ifdef HAVE_UVC
    if(!uri.scheme.compare("uvc")) {
        int vid = 0;
        int pid = 0;
        std::istringstream(uri.Get<std::string>("vid","0x0000")) >> std::hex >> vid;
        std::istringstream(uri.Get<std::string>("pid","0x0000")) >> std::hex >> pid;
        const unsigned int dev_id = uri.Get<int>("num",0);
        const ImageDim dim = uri.Get<ImageDim>("size", ImageDim(640,480));
        const unsigned int fps = uri.Get<unsigned int>("fps", 30);
        video = new UvcVideo(vid,pid,0,dev_id,dim.x,dim.y,fps);
    }else
#endif
#ifdef HAVE_DEPTHSENSE
    if(!uri.scheme.compare("depthsense")) {
        DepthSenseSensorType img1 = depthsense_sensor(uri.Get<std::string>("img1", "depth"));
        DepthSenseSensorType img2 = depthsense_sensor(uri.Get<std::string>("img2", ""));

        const ImageDim dim1 = uri.Get<ImageDim>("size1", img1 == DepthSenseDepth ? ImageDim(320, 240) : ImageDim(640, 480) );
        const ImageDim dim2 = uri.Get<ImageDim>("size2", img2 == DepthSenseDepth ? ImageDim(320, 240) : ImageDim(640, 480) );

        const unsigned int fps1 = uri.Get<unsigned int>("fps1", 30);
        const unsigned int fps2 = uri.Get<unsigned int>("fps2", 30);

        video = DepthSenseContext::I().GetDepthSenseVideo(0, img1, img2, dim1, dim2, fps1, fps2, uri);
    }else
#endif
#ifdef HAVE_TELICAM
    if (!uri.scheme.compare("teli")) {
        if (uri.Contains("roi")) {
            video = new TeliVideo(uri.Get<ImageRoi>("roi", ImageRoi(0,0,1920,1200) ) );
        }else{
            video = new TeliVideo();
        }
    }else
#endif
#ifdef HAVE_PLEORA
    if (!uri.scheme.compare("pleora")) {
        const std::string model_name = uri.Get<std::string>("model", "");
        const std::string serial_num = uri.Get<std::string>("sn", "");
        const size_t idx = uri.Get<size_t>("idx",0);
        const size_t bpp = uri.Get<size_t>("bpp",8);
        const size_t binx = uri.Get<size_t>("binx",1);
        const size_t biny = uri.Get<size_t>("biny",1);
        const size_t buffer_count = uri.Get<size_t>("buffers", PleoraVideo::DEFAULT_BUFFER_COUNT);
        const ImageDim desired_size = uri.Get<ImageDim>("size", ImageDim(0,0));
        const ImageDim desired_pos  = uri.Get<ImageDim>("pos", ImageDim(0,0));
        const size_t again = uri.Get<size_t>("again",-1);
        const double exposure = uri.Get<size_t>("exposure",0);
        const bool ext_trig = uri.Get<bool>("eTrig",false);
        const size_t analog_black_level= uri.Get<size_t>("abl",0);
        const bool use_separate_thread = uri.Get<bool>("use_separate_thread",false);
        const bool get_temperature = uri.Get<bool>("get_temperature",false);

        video = new PleoraVideo(
            model_name.empty() ? 0 : model_name.c_str(),
            serial_num.empty() ? 0 : serial_num.c_str(),
            idx, bpp, binx, biny, buffer_count,
            desired_size.x, desired_size.y, desired_pos.x, desired_pos.y,
            again, exposure, ext_trig, analog_black_level, use_separate_thread,
            get_temperature
        );
    }else
#endif
#ifdef _UNIX_
    if (!uri.scheme.compare("shmem")) {
        const ImageDim dim = uri.Get<ImageDim>("size", ImageDim(0, 0));
        const std::string sfmt = uri.Get<std::string>("fmt", "GRAY8");
        const VideoPixelFormat fmt = VideoFormatFromString(sfmt);
        const std::string shmem_name = std::string("/") + uri.url;
        boostd::shared_ptr<SharedMemoryBufferInterface> shmem_buffer =
            open_named_shared_memory_buffer(shmem_name, true);
        if (dim.x == 0 || dim.y == 0 || !shmem_buffer) {
            throw VideoException("invalid shared memory parameters");
        }

        const std::string cond_name = shmem_name + "_cond";
        boostd::shared_ptr<ConditionVariableInterface> buffer_full =
            open_named_condition_variable(cond_name);

        video = new SharedMemoryVideo(dim.x, dim.y, fmt, shmem_buffer,
            buffer_full);
    } else
#endif
    {
        throw VideoException("No known video handler for URI '" + uri.scheme + "'");
    }

    return video;
}

void RegisterScheme(std::string scheme,
    boostd::function<VideoInterface*(const Uri& uri)>& factory)
{
    ToLower(scheme);
    if (s_RegisteredUriSchemes.count(scheme) != 0) {
        throw VideoException("scheme " + scheme + " is already registered");
    }

    s_RegisteredUriSchemes[scheme] = factory;
}

VideoInput::VideoInput()
{
}

VideoInput::VideoInput(const std::string& uri)
{
    Open(uri);
}

VideoInput::~VideoInput()
{
    Close();
}

void VideoInput::Open(const std::string& sUri)
{
    uri = ParseUri(sUri);
    
    Close();
    
    // Create video device
    videos.push_back(OpenVideo(uri));
}

void VideoInput::Reset()

{
    Close();

    // Create video device
    videos.push_back(OpenVideo(uri));
}

void VideoInput::Close()
{
    for(size_t v=0; v< videos.size(); ++v) {
        delete videos[v];
    }
    videos.clear();
}

size_t VideoInput::SizeBytes() const
{
    if( !videos.size() ) throw VideoException("No video source open");
    return videos[0]->SizeBytes();
}

const std::vector<StreamInfo>& VideoInput::Streams() const
{
    if( !videos.size() ) throw VideoException("No video source open");
    return videos[0]->Streams();
}

unsigned int VideoInput::Width() const
{
    if( !videos.size() ) throw VideoException("No video source open");
    return (unsigned int)videos[0]->Streams()[0].Width();
}

unsigned int VideoInput::Height() const
{
    if( !videos.size() ) throw VideoException("No video source open");
    return (unsigned int)videos[0]->Streams()[0].Height();
}

VideoPixelFormat VideoInput::PixFormat() const
{
    if( !videos.size() ) throw VideoException("No video source open");
    return Streams()[0].PixFormat();
}

const Uri& VideoInput::VideoUri() const
{
    return uri;
}

void VideoInput::Start()
{
    if( !videos.size() ) throw VideoException("No video source open");
    videos[0]->Start();
}

void VideoInput::Stop()
{
    if( !videos.size() ) throw VideoException("No video source open");
    videos[0]->Stop();
}

bool VideoInput::GrabNext( unsigned char* image, bool wait )
{
    if( !videos.size() ) throw VideoException("No video source open");
    return videos[0]->GrabNext(image,wait);
}

bool VideoInput::GrabNewest( unsigned char* image, bool wait )
{
    if( !videos.size() ) throw VideoException("No video source open");
    return videos[0]->GrabNewest(image,wait);
}

bool VideoInput::Grab( unsigned char* buffer, std::vector<Image<unsigned char> >& images, bool wait, bool newest)
{
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

std::vector<VideoInterface*>& VideoInput::InputStreams()
{
    return videos;
}

}
