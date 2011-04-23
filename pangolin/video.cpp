#include "video.h"

#include "firewire.h"
#include "v4l.h"
#include "ffmpeg.h"

#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>
#define foreach BOOST_FOREACH

using namespace std;
using namespace boost;

namespace pangolin
{

struct VideoUri
{
    string scheme;
    string url;
    map<string,string> params;
};

ostream& operator<< (ostream &out, VideoUri &uri)
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
    : video(0)
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

VideoUri ParseUri(string str_uri)
{
    VideoUri uri;

    // extract scheme
    size_t ns = str_uri.find("://");
    if( ns == string::npos )
    {
        uri.scheme = "file";
        ns = 0;
    }else{
        uri.scheme = str_uri.substr(0,ns);
        ns += 3;
    }

    // extract url
    size_t nq = str_uri.find_first_of('?',ns);
    if(nq == string::npos)
    {
        uri.url = str_uri.substr(ns);
    }else{
        string queries = str_uri.substr(nq+1);
        uri.url = str_uri.substr(ns,nq-ns);
        vector<string> params;
        split(params, queries, boost::is_any_of("&"));
        foreach(string p, params)
        {
            vector<string> args;
            split(args, p, boost::is_any_of("=") );
            if( args.size() == 2 )
            {
                uri.params[args[0]] = args[1];
            }
        }
    }

    return uri;
}

void VideoInput::Open(std::string str_uri)
{
    VideoUri uri = ParseUri(str_uri);

    if(video) {
        delete video;
        video = 0;
    }

    if(!uri.scheme.compare("file")) {
        video = new FfmpegVideo(uri.url.c_str());
    }else if(!uri.scheme.compare("v4l")) {
        video = new V4lVideo(uri.url.c_str());
    }else if(!uri.scheme.compare("firewire")) {
        video = new FirewireVideo();
    }else{
        throw VideoException("Unable to open video URI");
    }
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
