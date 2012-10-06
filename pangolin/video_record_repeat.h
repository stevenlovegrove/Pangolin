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

#ifndef PANGOLIN_VIDEO_RECORD_REPEAT_H
#define PANGOLIN_VIDEO_RECORD_REPEAT_H

#include "pangolin.h"

#include "video.h"
#include "video_recorder.h"

namespace pangolin
{

struct VideoRecordRepeat : public VideoInterface
{
    VideoRecordRepeat(
        std::string uri, std::string save_filename, int buffer_size_bytes
    );
    ~VideoRecordRepeat();

    unsigned Width() const;
    unsigned Height() const;
    size_t SizeBytes() const;
    std::string PixFormat() const;

    void Start();
    void Stop();
    bool GrabNext( unsigned char* image, bool wait = true );
    bool GrabNewest( unsigned char* image, bool wait = true );

    void Record();
    void Play(bool realtime = true);
    void Source();

    int FrameId();

    bool IsRecording() const;
    bool IsPlaying() const;

protected:
    VideoInterface* video_src;
    VideoInterface* video_file;
    VideoRecorder* video_recorder;

    std::string filename;
    int buffer_size_bytes;

    int frame_num;
};

}

#endif // PANGOLIN_VIDEO_RECORD_REPEAT_H
