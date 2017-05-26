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
#include <condition_variable>
#include <mutex>
#include <queue>

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

    SyncTime()
    {
        SetOffset(std::chrono::milliseconds(0));
    }

    // No copy constructor
    SyncTime(const SyncTime&) = delete;

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

    int64_t QueueEvent(int64_t event_time_us)
    {
        std::lock_guard<std::mutex> l(time_mutex);

        if(event_time_us) {
            time_queue_us.push(event_time_us);
        }

        return event_time_us;
    }

    int64_t WaitDequeueAndQueueEvent(int64_t event_time_us, int64_t new_event_time_us =0 )
    {
        std::unique_lock<std::mutex> l(time_mutex);

        if(event_time_us) {
            PANGO_ENSURE(time_queue_us.size());

            // Wait until we're top the priority-queue
            queue_changed.wait(l, [&](){
                return time_queue_us.top() == event_time_us;
            });
            // Dequeue
            time_queue_us.pop();
        }

        if(new_event_time_us) {
            // Add the new event whilst we still hold the lock, so that our
            // event can't be missed
            time_queue_us.push(new_event_time_us);

            if(time_queue_us.top() == new_event_time_us) {
                // Return to avoid yielding when we're next.
                return new_event_time_us;
            }
        }

        // Only yield if another device is next
        queue_changed.notify_all();
        return new_event_time_us;
    }

private:
    std::priority_queue<int64_t> time_queue_us;
    Duration virtual_offset;
    std::mutex time_mutex;
    std::condition_variable queue_changed;
};

struct SyncTimeEventPromise
{
    SyncTimeEventPromise(SyncTime& sync, int64_t time_us = 0)
        : sync(sync), time_us(time_us)
    {
        sync.QueueEvent(time_us);
    }

    ~SyncTimeEventPromise()
    {
        if(time_us) {
            sync.WaitDequeueAndQueueEvent(time_us);
        }
    }

    void WaitAndRenew(int64_t new_time_us)
    {
        time_us = sync.WaitDequeueAndQueueEvent(time_us, new_time_us);
    }

private:
    SyncTime& sync;
    int64_t time_us;
};

}
