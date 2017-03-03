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

#include <mutex>
#include <fstream>
#include <thread>

#include <pangolin/utils/timer.h>
#include <pangolin/utils/file_utils.h>
#include <pangolin/log/sync_time.h>
#include <pangolin/log/packetstream.h>
#include <pangolin/log/packetstream_source.h>
#include <pangolin/log/packet_index.h>

namespace pangolin
{

class PANGOLIN_EXPORT PacketStreamReader
{
public:
    // Contains information about the queued-up frame. Relies on RVO.
    // Obtained from nextFrame() function
	struct FrameInfo
	{        
        FrameInfo()
            : src(static_cast<decltype(src)>(-1)), time(-1),
              size(static_cast<decltype(size)>(-1)),
              sequence_num(static_cast<decltype(sequence_num)>(-1))
        {
        }

        bool None() const
        {
            return src == static_cast<decltype(src)>(-1);
        }

        operator bool() const
        {
            return !None();
        }

        PacketStreamSourceId src;
        int64_t time;
        size_t size;
        size_t sequence_num;
        json::value meta;

        // The 'frame' includes the json and the packet.
        std::streampos frame_streampos;
        std::streampos packet_streampos;
	};

    PacketStreamReader()
        : _starttime(0)
    {
    }

    PacketStreamReader(const std::string& filename)
        : _stream(filename), _starttime(0)
    {
        Init();
    }

    ~PacketStreamReader()
    {
        Close();
    }

    void Open(const std::string& filename)
    {
        _starttime = 0;
        _stream.open(filename);
        Init();
    }

    void Close() {
        _stream.close();
        _sources.clear();
    }

    const SourceIndexType& Sources() const
    {
        return _sources;
    }

    // User is responsible for locking, since we cannot know the desired
    // locking behaviour
    void Lock()
    {
        _mutex.lock();
    }

    void Unlock()
    {
        _mutex.unlock();
    }

    // Exposes the underlying mutex... this allows std::lock_guard,
    // and similar constructs.
    std::recursive_mutex& Mutex()
    {
        return _mutex;
    }

    FrameInfo NextFrame(PacketStreamSourceId src, SyncTime *sync);

    size_t ReadRaw(char* target, size_t len);

    size_t Skip(size_t len);

    bool Good() const
    {
        return _stream.good();
    }

    // Returns the current frame for source
    size_t GetPacketIndex(PacketStreamSourceId src_id) const;

    inline size_t GetNumPackets(PacketStreamSourceId src_id) const
    {
        return _index.packetCount(src_id);
    }

    // Jumps to a particular packet.
    // If the address of a SyncTime is passed in, the object will be updated
    // to maintain synchronization after the seek is complete.
    FrameInfo Seek(PacketStreamSourceId src, size_t framenum, SyncTime *sync = nullptr);

private:
    class Stream: public std::ifstream
    {
    public:
        Stream()
            : _seekable(false)
        {
            cclear();
        }

        Stream(const std::string& filename)
            : Base(filename.c_str(), std::ios::in | std::ios::binary),
              _seekable(!IsPipe(filename))
        {
            cclear();
        }

        bool seekable() const
        {
            return _seekable;
        }

        size_t data_len() const
        {
            return _data_len;
        }

        void data_len(size_t d)
        {
            _data_len = d;
        }

        void open(const std::string& filename)
        {
            close();
            Base::open(filename.c_str(), std::ios::in | std::ios::binary);
            _seekable = !IsPipe(filename);
        }

        void close()
        {
            cclear();
            if (Base::is_open()) Base::close();
        }

        void seekg(std::streampos target);

        void seekg(std::streamoff off, std::ios_base::seekdir way);

        std::streampos tellg();

        size_t read(char* target, size_t len);

        char get();

        size_t skip(size_t len);

        size_t readUINT();

        int64_t readTimestamp();

        pangoTagType peekTag();

        pangoTagType readTag();

        pangoTagType readTag(pangoTagType);

        pangoTagType syncToTag();

        FrameInfo peekFrameHeader(const PacketStreamReader&);

        FrameInfo readFrameHeader(const PacketStreamReader&);

    private:
        using Base = std::ifstream;

        bool _seekable;
        pangoTagType _tag;
        FrameInfo _frame;

        // Amount of frame data left to read. Tracks our position within a data block.
        size_t _data_len;

        void cclear() {
            _data_len = 0;
            _tag = 0;
            _frame.src = static_cast<decltype(_frame.src)>(-1);
        }
    };


    void Init();

    void SetupIndex();

    void ParseHeader();

    void ParseNewSource();

    void ParseIndex();

    std::streampos ParseFooter();

	FrameInfo _nextFrame();

    void WaitForTimeSync(const SyncTime& timer, int64_t wait_for) const;

	void SkipSync();

    void ReSync() {
        _stream.syncToTag();
    }

    SourceIndexType _sources;
    std::vector<size_t> _next_packet_framenum;
    PacketIndex _index;

    Stream _stream;
    std::recursive_mutex _mutex;
    int64_t _starttime;
};








}




