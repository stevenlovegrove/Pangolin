#include "video_record_repeat.h"
#include "pvn_video.h"

namespace pangolin
{

VideoRecordRepeat::VideoRecordRepeat(
    std::string uri, std::string save_filename,
    int buffer_size_bytes, std::string var_record_prefix
    ) : video_src(0), video_file(0), video_recorder(0),
    filename(save_filename), buffer_size_bytes(buffer_size_bytes)
{
    video_src = OpenVideo(uri);
}

VideoRecordRepeat::~VideoRecordRepeat()
{
    if(video_recorder)
        delete video_recorder;
    if( video_src )
        delete video_src;
}

void VideoRecordRepeat::Record()
{
    if( video_recorder == 0 )
    {
        if(video_file) {
            delete video_file;
            video_file = 0;
        }

        video_recorder = new VideoRecorder(
            filename, video_src->Width(), video_src->Height(),
            "RGB24", buffer_size_bytes
        );

        video_src->Start();
    }
}

void VideoRecordRepeat::Play(bool realtime)
{
    if( video_file ) {
        delete video_file;
        video_file = 0;
    }

    video_src->Stop();

    if(video_recorder) {
        delete video_recorder;
        video_recorder = 0;
    }

    video_file = new PvnVideo(filename.c_str(),realtime);
}

void VideoRecordRepeat::Source()
{
    if(video_file) {
        delete video_file;
        video_file = 0;
    }

    if(video_recorder) {
        delete video_recorder;
        video_recorder = 0;
    }

    video_src->Start();
}

unsigned VideoRecordRepeat::Width() const
{
    if( !video_src ) throw VideoException("No video source open");
    return video_src->Width();

}

unsigned VideoRecordRepeat::Height() const
{
    if( !video_src ) throw VideoException("No video source open");
    return video_src->Height();
}

std::string VideoRecordRepeat::PixFormat() const
{
    if( !video_src ) throw VideoException("No video source open");
    return video_src->PixFormat();
}

void VideoRecordRepeat::Start()
{
//    video_src->Start();
}

void VideoRecordRepeat::Stop()
{
//    video_src->Stop();
}

bool VideoRecordRepeat::GrabNext( unsigned char* image, bool wait )
{
    if( video_recorder != 0 )
    {
        bool success = video_src->GrabNext(image,wait);
        if( success ) {
            video_recorder->RecordFrame(image);
        }
        return success;
    }else if( video_file != 0 ) {
        return video_file->GrabNext(image,wait);
    }else{
        return video_src->GrabNext(image,wait);
    }
}

bool VideoRecordRepeat::GrabNewest( unsigned char* image, bool wait )
{
    if( video_recorder != 0 )
    {
        bool success = video_src->GrabNewest(image,wait);
        if( success ) {
            video_recorder->RecordFrame(image);
        }
        return success;
    }else if( video_file != 0 ) {
        return video_file->GrabNewest(image,wait);
    }else{
        return video_src->GrabNewest(image,wait);
    }
}


}

