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

#include <atomic>

namespace pangolin
{

// Lightweight timestamp class to allow synchronized playback from the same (or a different) stream.
// All playback functions called with the same SyncTime will be time-synchronized, and will remain synchronized on seek() if the SyncTime is passed in when seeking.
// Playback with multiple SyncTimes (on the same or different streams) should also be synced, even in different processes or systems (underlying clock sync is not required).
// However, playback with multiple SyncTimes will break on seek().
class PANGOLIN_EXPORT SyncTime
{
public:
    using Clock = std::chrono::system_clock;
    using Duration = Clock::duration;
    using TimePoint = Clock::time_point;

    SyncTime(Duration virtual_clock_offset)
    {
        SetOffset(virtual_clock_offset);
    }

    void SetOffset(Duration virtual_clock_offset)
    {
        virtual_offset = virtual_clock_offset;
    }

    void SetClock(TimePoint virtual_now)
    {
        virtual_offset = virtual_now - Clock::now();
    }

    TimePoint TimeNow() const
    {
        return Clock::now() + virtual_offset;
    }

    TimePoint ToVirtual(TimePoint real) const
    {
        return real + virtual_offset;
    }

    TimePoint ToReal(TimePoint virt) const
    {
        return virt - virtual_offset;
    }

    void WaitUntil(TimePoint virtual_time) const
    {
        std::this_thread::sleep_until( ToReal(virtual_time) );
    }

private:
    std::chrono::system_clock::duration virtual_offset;
};

}
