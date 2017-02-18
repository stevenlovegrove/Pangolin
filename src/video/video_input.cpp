/* This file is part of the Pangolin Project.
 * http://github.com/stevenlovegrove/Pangolin
 *
 * Copyright (c) 2013 Steven Lovegrove
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

#include <pangolin/video/video_input.h>
#include <pangolin/video/video_output.h>

namespace pangolin
{

VideoInput::VideoInput()
    : frame_num(0), record_frame_skip(1), record_once(false), record_continuous(false)
{
}

VideoInput::VideoInput(
    const std::string& input_uri,
    const std::string& output_uri
    ) : frame_num(0), record_frame_skip(1), record_once(false), record_continuous(false)
{
    Open(input_uri, output_uri);
}

void VideoInput::Open(
    const std::string& input_uri,
    const std::string& output_uri
    ) 
{
    str_uri_input = input_uri;
    uri_input = ParseUri(input_uri);
    uri_output = ParseUri(output_uri);

    if (uri_output.scheme == "file") {
        // Default to pango output
        uri_output.scheme = "pango";
    }

    // Start off playing from video_src
    video_src = OpenVideo(input_uri);
    Source();
}

void VideoInput::Reset()
{
    Close();

    // Create video device
    video_src = OpenVideo(uri_input);
    Source();
}

void VideoInput::Close()
{
    video_src.release();
    videos.clear();
}

VideoInput::~VideoInput()
{
    Close();
}

const std::string& VideoInput::LogFilename() const
{
    return uri_output.url;
}

bool VideoInput::Grab( unsigned char* buffer, std::vector<Image<unsigned char> >& images, bool wait, bool newest)
{
    if( !video_src ) throw VideoException("No video source open");

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

void VideoInput::InitialiseRecorder()
{
    video_recorder.reset();
    video_file.reset();

    video_recorder = OpenVideoOutput(uri_output);
    video_recorder->SetStreams(
        video_src->Streams(), str_uri_input,
        GetVideoDeviceProperties(video_src.get())
    );
}

void VideoInput::Record()
{
    // Switch sub-video
    videos.resize(1);
    videos[0] = video_src.get();

    // Initialise recorder and ensure src is started
    InitialiseRecorder();
    video_src->Start();
    frame_num = 0;
    record_continuous = true;
}

void VideoInput::RecordOneFrame()
{
    // Append to existing video.
    if(!video_recorder) {
        InitialiseRecorder();
    }
    record_continuous = false;
    record_once = true;

    // Switch sub-video
    videos.resize(1);
    videos[0] = video_src.get();
}

void VideoInput::Play(bool realtime)
{
    video_file.reset();
    video_src->Stop();
    video_recorder.reset();

    video_file = OpenVideo(
        realtime ? "file:[realtime]//" + uri_output.url :
                   uri_output.url
    );

    frame_num = 0;

    // Switch sub-video
    videos.resize(1);
    videos[0] = video_file.get();
}

void VideoInput::Source()
{
    video_file.reset();
    video_recorder.reset();

    frame_num = 0;

    // Switch sub-video
    videos.resize(1);
    videos[0] = video_src.get();
    video_src->Start();
}

size_t VideoInput::SizeBytes() const
{
    if( !video_src ) throw VideoException("No video source open");
    return video_src->SizeBytes();
}

const std::vector<StreamInfo>& VideoInput::Streams() const
{
    return video_src->Streams();
}

void VideoInput::Start()
{
    video_src->Start();
}

void VideoInput::Stop()
{
    if(IsRecording()) {
        video_recorder.reset();
    }else{
        video_src->Stop();
    }
}

bool VideoInput::GrabNext( unsigned char* image, bool wait )
{
    frame_num++;

    const bool should_record = (record_continuous && !(frame_num % record_frame_skip)) || record_once;

    if( should_record && video_recorder != 0 ) {
        bool success = video_src->GrabNext(image, wait);
        if( success ) {
            video_recorder->WriteStreams(image, GetVideoFrameProperties(video_src.get()) );
            record_once = false;
        }
        return success;
    }else if( video_file != 0 ) {
        return video_file->GrabNext(image,wait);
    }else{
        return video_src->GrabNext(image,wait);
    }
}

bool VideoInput::GrabNewest( unsigned char* image, bool wait )
{
    frame_num++;

    const bool should_record = (record_continuous && !(frame_num % record_frame_skip)) || record_once;

    if( should_record && video_recorder != 0 )
    {
        bool success = video_src->GrabNewest(image,wait);
        if( success) {
            video_recorder->WriteStreams(image, GetVideoFrameProperties(video_src.get()) );
            record_once = false;
        }
        return success;
    }else if( video_file != 0 ) {
        return video_file->GrabNewest(image,wait);
    }else{
        return video_src->GrabNewest(image,wait);
    }
}

int VideoInput::FrameId()
{
    return frame_num;
}

void VideoInput::SetTimelapse(size_t one_in_n_frames)
{
    record_frame_skip = one_in_n_frames;
}

bool VideoInput::IsRecording() const
{
    return video_recorder != 0;
}

bool VideoInput::IsPlaying() const
{
    return video_file != 0;
}



}

