#include <pangolin/video_record_repeat.h>
#include <pangolin/video/pvn_video.h>
#include <pangolin/widgets.h>

namespace pangolin
{

VideoRecordRepeat::VideoRecordRepeat(
    std::string uri, std::string save_filename, int buffer_size_bytes
    ) : video_src(0), video_file(0), video_recorder(0),
    filename(save_filename), buffer_size_bytes(buffer_size_bytes), frame_num(0)
{
    video_src = OpenVideo(uri);
}

VideoRecordRepeat::~VideoRecordRepeat()
{
    if(video_recorder)
        delete video_recorder;
    if( video_src )
        delete video_src;
    if( video_file )
        delete video_file;
}

void VideoRecordRepeat::Record()
{
    if( video_recorder ) {
        video_src->Stop();
        delete video_recorder;
        video_recorder = 0;
    }

    if(video_file) {
        delete video_file;
        video_file = 0;
    }

    video_recorder = new VideoRecorder(
        filename, video_src->Width(), video_src->Height(),
        video_src->PixFormat(), buffer_size_bytes
    );

    video_src->Start();
    frame_num = 0;
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
    frame_num = 0;
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
    frame_num = 0;
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

size_t VideoRecordRepeat::SizeBytes() const
{
    if( !video_src ) throw VideoException("No video source open");
    return video_src->SizeBytes();
}

std::string VideoRecordRepeat::PixFormat() const
{
    if( !video_src ) throw VideoException("No video source open");
    return video_src->PixFormat();
}

void VideoRecordRepeat::Start()
{
    // Semantics of this?
//    video_src->Start();
}

void VideoRecordRepeat::Stop()
{
    // Semantics of this?
    if(video_recorder) {
        delete video_recorder;
        video_recorder = 0;
    }
}

bool VideoRecordRepeat::GrabNext( unsigned char* image, bool wait )
{
    frame_num++;

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
    frame_num++;

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

int VideoRecordRepeat::FrameId()
{
    return frame_num;
}

bool VideoRecordRepeat::IsRecording() const
{
    return video_recorder != 0;
}

bool VideoRecordRepeat::IsPlaying() const
{
    return video_file != 0;
}



}

