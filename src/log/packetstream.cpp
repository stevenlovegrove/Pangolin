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

#include <pangolin/log/packetstream.h>

namespace pangolin
{

PacketStreamWriter::PacketStreamWriter(const std::string& filename, unsigned int buffer_size_bytes )
    : buffer(filename, buffer_size_bytes), writer(&buffer), id_level(0)
{
    // Start of file magic
    WriteTag(TAG_PANGO_MAGIC);
    WritePangoHeader();
}

PacketStreamWriter::~PacketStreamWriter()
{
    WriteStats();
}

unsigned int PacketStreamWriter::AddSource(
    const std::string& type,
    const std::string& uri,
    const std::string& json_frame,
    const std::string& json_header,
    const std::string& json_aux_types
) {
    const std::string ns = type+"::";

    std::string err;
    picojson::value header;
    picojson::value typed_aux;
    picojson::value typed_frame;

    picojson::parse(header,json_header.begin(), json_header.end(), &err);
    if(!err.empty()) throw std::runtime_error("Frame header parse error: " + err);

    picojson::parse(typed_aux,json_aux_types.begin(), json_aux_types.end(), &err);
    if(!err.empty()) throw std::runtime_error("Frame types parse error: " + err);

    picojson::parse(typed_frame,json_frame.begin(), json_frame.end(), &err);
    if(!err.empty()) throw std::runtime_error("Frame definition parse error: " + err);

    if( typed_aux.is<picojson::object>() ) {
        typemap.AddTypes(ns, typed_aux.get<picojson::object>() );
        PacketStreamTypeId frame_id = typemap.CreateOrGetType(ns, typed_frame);
        typemap.AddAlias(ns + "pango_frame", frame_id);

        picojson::value json_src(picojson::object_type,false);
        json_src.get<picojson::object>()["_type_"] = picojson::value(type);
        json_src.get<picojson::object>()["_uri_"] = picojson::value(uri);
        json_src.get<picojson::object>()["header"] = header;
        json_src.get<picojson::object>()["typed_aux"] = typed_aux;
        json_src.get<picojson::object>()["typed_frame"] = typed_frame;

        WriteTag(TAG_ADD_SOURCE);
        json_src.serialize(std::ostream_iterator<char>(writer), true);

        const PacketStreamType& frame_type = typemap.GetType(ns + PANGO_FRAME);
        const size_t src_id = sources.size();
        sources.push_back( {type,uri,frame_type} );
        return src_id;
    }else{
        throw std::runtime_error("Frame definition must be JSON object.");
    }
}

void PacketStreamWriter::WriteSourceFrame(unsigned int src, char* data, size_t n)
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
    writer.write(data, n);
    bytes_written += n;
}

void PacketStreamWriter::WritePangoHeader()
{
    // Write Header
    picojson::value pango(picojson::object_type,false);
    pango.get<picojson::object>()["version"] = picojson::value(PANGOLIN_VERSION_STRING);
    pango.get<picojson::object>()["date_created"] = picojson::value("2014/08/04 14:28:38 GMT");

    WriteTag(TAG_PANGO_HDR);
    pango.serialize(std::ostream_iterator<char>(writer), true);
}

void PacketStreamWriter::WriteStats()
{
    WriteTag(TAG_PANGO_STATS);
    picojson::value stat(picojson::object_type,false);
    stat.get<picojson::object>()["num_sources"] = picojson::value((int64_t)sources.size());
    stat.get<picojson::object>()["bytes_written"] = picojson::value((int64_t)bytes_written);
    stat.serialize(std::ostream_iterator<char>(writer), true);
}

void PacketStreamWriter::WriteSync()
{
    for(int i=0; i<10; ++i) {
        WriteTag(TAG_PANGO_SYNC);
    }
}

}
