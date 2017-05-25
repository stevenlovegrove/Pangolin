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

#include <fstream>
#include <mutex>
#include <thread>

#include <pangolin/log/packet_index.h>
#include <pangolin/log/packetstream.h>
#include <pangolin/log/packetstream_source.h>
#include <pangolin/log/sync_time.h>
#include <pangolin/utils/file_utils.h>
#include <pangolin/utils/timer.h>

namespace pangolin
{

// Encapsulate serialized reading of Packet from stream.
struct Packet
{
    Packet(PacketStream& s, std::recursive_mutex& mutex, SourceIndexType& srcs)
        : _stream(s), lock(mutex)
    {
        ParsePacketHeader(s, srcs);
    }

    Packet(const Packet&) = delete;

    Packet(Packet&& o)
        : src(o.src), time(o.time), size(o.size), sequence_num(o.sequence_num),
          meta(std::move(o.meta)), frame_streampos(o.frame_streampos), _stream(o._stream),
          lock(std::move(o.lock)), _data_len(o._data_len)
    {
        o._data_len = 0;
    }

    ~Packet()
    {
        ReadRemaining();
    }

    size_t ReadRaw(char* target, size_t len)
    {
        PANGO_ENSURE(len <= _data_len);
        const size_t num_read = _stream.read(target, len);
        _data_len -= num_read;
        return num_read;
    }

    size_t Skip(size_t len)
    {
        PANGO_ENSURE(len <= _data_len);
        const size_t num_read = _stream.skip(len);
        _data_len -= num_read;
        return num_read;
    }

    PacketStreamSourceId src;
    int64_t time;
    size_t size;
    size_t sequence_num;
    picojson::value meta;
    std::streampos frame_streampos;

private:
    void ParsePacketHeader(PacketStream& s, SourceIndexType& srcs)
    {
        size_t json_src = -1;

        frame_streampos = s.tellg();
        if (s.peekTag() == TAG_SRC_JSON)
        {
            s.readTag(TAG_SRC_JSON);
            json_src = s.readUINT();
            picojson::parse(meta, s);
        }

        s.readTag(TAG_SRC_PACKET);
        time = s.readTimestamp();

        src = s.readUINT();
        PANGO_ENSURE(json_src == size_t(-1) || json_src == src, "Frame preceded by metadata for a mismatched source. Stream may be corrupt.");

        PacketStreamSource& src_packet = srcs[src];

        size = src_packet.data_size_bytes;
        if (!size) {
            size = s.readUINT();
        }
        sequence_num = src_packet.next_packet_id++;
        _data_len = size;
    }

    void ReadRemaining()
    {
        while(_data_len) {
            Skip(_data_len);
        }
    }

    PacketStream& _stream;
    std::unique_lock<std::recursive_mutex> lock;
    size_t _data_len;
};

class PANGOLIN_EXPORT PacketStreamReader
{
public:
    PacketStreamReader();

    PacketStreamReader(const std::string& filename);

    ~PacketStreamReader();

    void Open(const std::string& filename);

    void Close();

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

    Packet NextFrame(PacketStreamSourceId src);

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
    size_t Seek(PacketStreamSourceId src, size_t framenum);

private:

    bool GoodToRead();

    void SetupIndex();

    void ParseHeader();

    void ParseNewSource();

    void ParseIndex();

    std::streampos ParseFooter();

    Packet NextFrame();

    void SkipSync();

    void ReSync() {
        _stream.syncToTag();
    }

    std::string _filename;
    SourceIndexType _sources;
    PacketIndex _index;
    SyncTime::TimePoint packet_stream_start;

    PacketStream _stream;
    std::recursive_mutex _mutex;

    bool _is_pipe;
    int _pipe_fd;
};








}




