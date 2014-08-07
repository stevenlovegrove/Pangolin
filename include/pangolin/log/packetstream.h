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

namespace pangolin
{

const static std::string PANGO_FRAME = "pango_frame";

const static std::string TAG_PANGO_MAGIC = "PA";
const static std::string TAG_PANGO_HDR   = "NGO";
const static std::string TAG_PANGO_SYNC  = "SYC";
const static std::string TAG_PANGO_STATS = "STA";
const static std::string TAG_ADD_SOURCE  = "SRC";
const static std::string TAG_SRC_FRAME   = "FRM";

struct PANGOLIN_EXPORT PacketStreamSource
{
    std::string type;
    std::string name;
    const PacketStreamType& frametype;
};

class PANGOLIN_EXPORT PacketStreamWriter
{
public:

    PacketStreamWriter(const std::string& filename, unsigned int buffer_size_bytes = 10000000);

    ~PacketStreamWriter();

    unsigned int AddSource(
        const std::string& type,
        const std::string& uri,
        const std::string& json_frame = "{}",
        const std::string& json_header = "{}",
        const std::string& json_aux_types = "{}"
    );

    void WriteSourceFrame(unsigned int src, char* data, size_t n);

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
    inline void WriteCompressedInt(size_t n)
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
    unsigned int bytes_written;
    threadedfilebuf buffer;
    std::ostream writer;
    int id_level;
};

}

#endif // PANGOLIN_PACKETSTREAM_H
