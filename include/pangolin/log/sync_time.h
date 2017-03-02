/* This file is part of the Pangolin Project.
 * http://github.com/stevenlovegrove/Pangolin
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

#pragma once

#include <pangolin/platform.h>
#include <pangolin/utils/timer.h>

namespace pangolin
{

// Lightweight timestamp class to allow synchronized playback from the same (or a different) stream.
// All playback functions called with the same SyncTime will be time-synchronized, and will remain synchronized on seek() if the SyncTime is passed in when seeking.
// Playback with multiple SyncTimes (on the same or different streams) should also be synced, even in different processes or systems (underlying clock sync is not required).
// However, playback with multiple SyncTimes will break on seek().
class PANGOLIN_EXPORT SyncTime
{
public:
    SyncTime()
        :_start(TimeNow_us())
    {
    }

    void Start(){
        std::lock_guard<std::mutex> lg(_startlock);_start = TimeNow_us();
    }

    void WaitUntilOffset(int64_t stream_time_offset) const
        {
            const auto viewer_time_offset = TimeNow_us() - _start;
        if (viewer_time_offset < stream_time_offset) {
            std::this_thread::sleep_for(
                std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::microseconds(stream_time_offset - viewer_time_offset)
                )
            );
        }
    }

    void ResyncToOffset(int64_t stream_time_offset) {
        std::lock_guard<std::mutex> lg(_startlock);_start = TimeNow_us() - stream_time_offset;
    }

private:
    int64_t _start;
    std::mutex _startlock;
};

}
