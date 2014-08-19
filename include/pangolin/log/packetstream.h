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
#include <pangolin/compat/condition_variable.h>


namespace pangolin
{

const static std::string PANGO_FRAME = "pango_frame";

const unsigned int TAG_LENGTH = 3;
const static std::string TAG_PANGO_MAGIC = "PA";
const static std::string TAG_PANGO_HDR   = "NGO";
const static std::string TAG_PANGO_SYNC  = "SYC";
const static std::string TAG_PANGO_STATS = "STA";
const static std::string TAG_ADD_SOURCE  = "SRC";
const static std::string TAG_SRC_FRAME   = "FRM";
const static std::string TAG_END         = "END";

struct PANGOLIN_EXPORT PacketStreamSource
{
    std::string type;
    std::string uri;
    picojson::value header;
    PacketStreamTypeId frametype;
    bool registered_handler;
};

typedef unsigned int PacketStreamSourceId;

class PANGOLIN_EXPORT PacketStreamWriter
{
public:

    PacketStreamWriter(const std::string& filename, unsigned int buffer_size_bytes = 10000000);

    ~PacketStreamWriter();

    PacketStreamSourceId AddSource(
        const std::string& type,
        const std::string& uri,
        const std::string& json_frame = "{}",
        const std::string& json_header = "{}",
        const std::string& json_aux_types = "{}"
    );

    void WriteSourceFrame(PacketStreamSourceId src, char* data, size_t n);

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

    inline void WriteTag(const std::string& tag)
    {
        writer << tag;
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
    const static int MAX_SOURCES = 20;

    struct NewSourceReceiver
    {
        virtual void NewSource(PacketStreamSourceId src_id, const PacketStreamSource& src) = 0;
    };

    struct SourceFrame
    {
        PacketStreamSourceId src_id;
        size_t size_bytes;
        std::ifstream& reader;
    };

    PacketStreamReader(const std::string& filename);
    ~PacketStreamReader();

    void RegisterSourceHeaderHandler( NewSourceReceiver& receiver );
    void RegisterFrameHandler( PacketStreamSourceId src_id );
    void UnregisterFrameHandler( PacketStreamSourceId src_id );
    bool ProcessUpToNextSourceFrame(PacketStreamSourceId src_id);

    int ProcessMessagesUntilRegisteredSourceFrame();
    bool ReadTag();

    inline std::basic_istream<char>& Read(char* s, size_t n)
    {
        return reader.read(s,n);
    }

protected:
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

    void ReadHeaderPacket();
    void ReadNewSourcePacket();
    void ReadStatsPacket();
    void ReadOverSourceFramePacket(PacketStreamSourceId src_id);
    unsigned char next_tag[4];

    PacketStreamTypeMap typemap;
    std::vector<PacketStreamSource> sources;
    boostd::condition_variable source_tag_available[MAX_SOURCES];

    std::ifstream reader;

    std::vector<NewSourceReceiver*> source_handlers;
//    std::map<unsigned int, FrameHandler> frame_handlers;
};

}

#endif // PANGOLIN_PACKETSTREAM_H
