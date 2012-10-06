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

#ifndef PANGOLIN_VIDEO_RECORDER_H
#define PANGOLIN_VIDEO_RECORDER_H

#include <vector>
#include <string>

#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>

#include "video/pvn_video.h"
#include "threadedfilebuf.h"

namespace pangolin
{
    struct VideoRecorderException : std::exception
    {
        VideoRecorderException(std::string str) : desc(str) {}
        VideoRecorderException(std::string str, std::string detail) {
            desc = str + "\n\t" + detail;
        }
        ~VideoRecorderException() throw() {}
        const char* what() const throw() { return desc.c_str(); }
        std::string desc;
    };

    class VideoRecorder
    {
    public:
        VideoRecorder(
            const std::string& filename,
            int stream0_width, int stream0_height, std::string stream0_fmt,
            unsigned int buffer_size_bytes = 1024*1024*100
        );
        ~VideoRecorder();

        // Save img (with correct format and resolution) to video, returning video frame id.
        int RecordFrame(void* img);

        void operator()();

    protected:       
        int frames;
        std::vector<VideoStream> stream_info;

        threadedfilebuf buffer;
        std::ostream writer;

        void WriteFileHeader();
    };
}

#endif // PANGOLIN_VIDEO_RECORDER_H
