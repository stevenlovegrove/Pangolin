/* This file is part of the Pangolin Project.
 * http://github.com/stevenlovegrove/Pangolin
 *
 * Copyright (c) 2014 Steven Lovegrove
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

#ifndef PANGOLIN_PACKETSTREAM_H
#define PANGOLIN_PACKETSTREAM_H

#include <pangolin/platform.h>
#include <pangolin/utils/threadedfilebuf.h>
#include <pangolin/log/packetstreamtypemap.h>
#include <pangolin/compat/function.h>
#include <pangolin/compat/mutex.h>
#include <pangolin/compat/condition_variable.h>
#include <stdint.h>


namespace pangolin
{

const static std::string PANGO_MAGIC = "PANGO";
const static std::string PANGO_FRAME = "PANGO_FRAME";

const unsigned int TAG_LENGTH = 3;

#define PANGO_TAG(a,b,c) ( (c<<16) | (b<<8) | a)
const uint32_t TAG_PANGO_HDR   = PANGO_TAG('L', 'I', 'N');
const uint32_t TAG_PANGO_SYNC  = PANGO_TAG('S', 'Y', 'N');
const uint32_t TAG_PANGO_STATS = PANGO_TAG('S', 'T', 'A');
const uint32_t TAG_ADD_SOURCE  = PANGO_TAG('S', 'R', 'C');
const uint32_t TAG_SRC_FRAME   = PANGO_TAG('F', 'R', 'M');
const uint32_t TAG_END         = PANGO_TAG('E', 'N', 'D');
#undef PANGO_TAG

struct PANGOLIN_EXPORT PacketStreamSource
{
    std::string type;
    std::string uri;
    picojson::value header;
    PacketStreamTypeId frametype;
};

typedef unsigned int PacketStreamSourceId;

PANGOLIN_EXPORT
double LoggingSystemTimeSeconds();

PANGOLIN_EXPORT
void ResetLoggingSystemTimeSeconds(double time_s = 0);

class PANGOLIN_EXPORT PacketStreamWriter
{
public:
    PacketStreamWriter();
    PacketStreamWriter(const std::string& filename, unsigned int buffer_size_bytes = 10000000);
    void Open(const std::string& filename, unsigned int buffer_size_bytes = 10000000);


    ~PacketStreamWriter();

    PacketStreamSourceId AddSource(
        const std::string& type,
        const std::string& uri,
        const std::string& json_frame = "{}",
        const std::string& json_header = "{}",
        const std::string& json_aux_types = "{}"
    );

    void WriteSourceFrame(PacketStreamSourceId src, const char* data, size_t n);

    void WritePangoHeader();

    void WriteStats();

    void WriteSync();

    inline const PacketStreamType& GetType(const std::string& name)
    {
        return typemap.GetType(name);
    }

    inline const PacketStreamType& GetFrameType()
    {
        return GetType(PANGO_FRAME);
    }

protected:
    inline void WriteCompressedUnsignedInt(size_t n)
    {
        while(n >= 0x80) {
            writer.put( 0x80 | (n & 0x7F) );
            n >>= 7;
        }
        writer.put( n );
    }

    inline void WriteTimestamp()
    {
        const double time_s = LoggingSystemTimeSeconds();
        writer.write((char*)&time_s, sizeof(double));
    }

    inline void WriteTag(const uint32_t tag)
    {
        writer.write((char*)&tag, TAG_LENGTH);
    }

    PacketStreamTypeMap typemap;
    std::vector<PacketStreamSource> sources;
    threadedfilebuf buffer;
    std::ostream writer;

    unsigned int bytes_written;
};

class PANGOLIN_EXPORT PacketStreamReader
{
public:
    ~PacketStreamReader();
    PacketStreamReader();
    PacketStreamReader(const std::string& filename, bool realtime = true);

    void Open(const std::string& filename, bool realtime = true);
    void Close();

    inline const std::vector<PacketStreamSource>& Sources() const
    {
        return sources;
    }

    bool ReadToSourceFrameAndLock(PacketStreamSourceId src_id);

    void ReleaseSourceFrameLock(PacketStreamSourceId src_id);

    // Should only read once lock is aquired
    inline std::basic_istream<char>& Read(char* s, size_t n)
    {
        return reader.read(s,n);
    }

protected:
    inline double ReadTimestamp()
    {
        double time_s;
        reader.read((char*)&time_s, sizeof(double));
        return time_s;
    }

    inline size_t ReadCompressedUnsignedInt()
    {
        size_t n = 0;
        size_t v = reader.get();
        while( v & 0x80 ) {
            n |= v & 0x7F;
            n <<= 7;
            v = reader.get();
        }
        return n|v;
    }

    void ProcessMessage();
    void ProcessMessagesUntilSourceFrame(int& nxt_src_id, double& time_s);

    bool ReadTag();
    void ReadHeaderPacket();
    void ReadNewSourcePacket();
    void ReadStatsPacket();
    void ReadOverSourceFramePacket(PacketStreamSourceId src_id);
    uint32_t next_tag;

    PacketStreamTypeMap typemap;
    std::vector<PacketStreamSource> sources;

    std::ifstream reader;
    boostd::mutex read_mutex;
    boostd::condition_variable new_frame;

    int frames;
    bool realtime;
};

}

#endif // PANGOLIN_PACKETSTREAM_H
