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
#include <pangolin/threadedfilebuf.h>
#include <pangolin/log/packetstreamtypemap.h>

namespace pangolin
{

const static std::string PANGO_FRAME = "pango_frame";

const static std::string TAG_PANGO_START = "PNGO\n";
const static std::string TAG_PANGO_SYNC  = "SYNC\n";
const static std::string TAG_PANGO_STATS = "STAT\n";
const static std::string TAG_ADD_SOURCE  = "ADDS\n";
const static std::string TAG_SRC_FRAME   = "SFRM\n";

struct PacketStreamSource
{
    std::string type;
    std::string name;
    const PacketStreamType& frametype;
};

class PANGOLIN_EXPORT PacketStreamWriter
{
public:

    PacketStreamWriter(const std::string& filename, unsigned int buffer_size_bytes)
        : buffer(filename, buffer_size_bytes), writer(&buffer), id_level(0)
    {
        StartFile();
    }

    ~PacketStreamWriter()
    {
        WriteStats();
    }

    unsigned int AddSource(
        const std::string& type,
        const std::string& name,
        const std::string& header,
        const std::string& frame_definition
    ) {
        const std::string ns = type+"::";
        typemap.AddTypes(ns, frame_definition);


        WriteTag(TAG_ADD_SOURCE);
        {
            Block();
            Line() << "\"type\":\"" << type << "\",\n";
            Line() << "\"name\":\"" << name << "\",\n";
            Line() << "\"header\":" << header << ",\n";
            Line() << "\"definition\":" << frame_definition << "\n";
            UnBlock();
        }

        const PacketStreamType& frame_type = typemap.GetType(ns + PANGO_FRAME);
        const size_t src_id = sources.size();
        sources.push_back( {type,name,frame_type} );
        return src_id;
    }

    void WriteSourceFrame(unsigned int src, char* data, size_t n)
    {
        // Write SOURCE_FRAME tag and source id
        WriteTag(TAG_SRC_FRAME);
        WriteCompressedInt(src);

        // Write frame size if dynamic so it can be skipped over easily
        const PacketStreamType& frametype = sources[src].frametype;
        if(!frametype.IsFixedSize()) {
            WriteCompressedInt(n);
        }else if(frametype.size_bytes != n) {
            throw std::runtime_error("Attempting to write frame of wrong size");
        }

        // Write data
        std::cout << "Writing " << n << " bytes" << std::endl;
        writer.write(data, n);
        bytes_written += n;
        std::cout << "done" << std::endl;
    }

    std::ostream& Writer()
    {
        return writer;
    }

    void WriteCompressedInt(size_t n)
    {
        while(n >= 0x80) {
            writer.put( 0x80 | (n & 0x7F) );
            n >>= 7;
        }
        writer.put( n );
    }

    void WriteStats()
    {
        WriteTag(TAG_PANGO_STATS);
        {
            Block();
            Line() << "\"num_sources\":\"" << sources.size() << "\",\n";
            Line() << "\"bytes_written\":\"" << bytes_written << "\",\n";
            UnBlock();
        }
    }

    void WriteSync()
    {
        for(int i=0; i<10; ++i) {
            WriteTag(TAG_PANGO_SYNC);
        }
    }

    const PacketStreamType& GetType(const std::string& name)
    {
        return typemap.GetType(name);
    }

protected:
    void StartFile()
    {
        // Start of file magic
        WriteTag(TAG_PANGO_START);
        {
            Block();
            Line() << "\"version\":\"" << PANGOLIN_VERSION_STRING << "\",\n";
            Line() << "\"date_created\":\"" << "2014/08/04 14:28:38 GMT" << "\"\n";
            UnBlock();
        }
    }

    void WriteTag(const std::string& tag)
    {
        writer << tag;
    }

    void Block()
    {
        writer << "{\n";
        id_level += 2;
    }

    void UnBlock()
    {
        id_level -= 2;
        writer << "}\n";
    }

    std::ostream& Line()
    {
        for(int i=0; i<id_level; ++i) {
            writer.put(' ');
        }
        return writer;
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
