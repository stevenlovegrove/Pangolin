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
#include <pangolin/utils/timer.h>
#include <pangolin/compat/thread.h>

#include <iostream>
#include <string>
#include <stdio.h>
#include <time.h>

namespace pangolin
{

const char* json_hdr_type        = "_type_";
const char* json_hdr_uri         = "_uri_";
const char* json_hdr_header      = "header";
const char* json_hdr_typed_aux   = "typed_aux";
const char* json_hdr_typed_frame = "typed_frame";

//////////////////////////////////////////////////////////////////////////
// Timer utils
//////////////////////////////////////////////////////////////////////////

int playback_devices = 0;
basetime startup_time = TimeNow();

double LoggingSystemTimeSeconds()
{
    return TimeDiff_s(startup_time, TimeNow() );
}

void ResetLoggingSystemTimeSeconds(double time_s)
{
    startup_time = TimeFromSeconds(TimeNow_s() - time_s);
}

//////////////////////////////////////////////////////////////////////////
// PacketStreamWriter
//////////////////////////////////////////////////////////////////////////

PacketStreamWriter::PacketStreamWriter()
    : writer(&buffer), bytes_written(0)
{
}

PacketStreamWriter::PacketStreamWriter(const std::string& filename, unsigned int buffer_size_bytes )
    : buffer(filename, buffer_size_bytes), writer(&buffer), bytes_written(0)
{
    // Start of file magic
    writer.write(PANGO_MAGIC.c_str(), PANGO_MAGIC.size());

    WritePangoHeader();
}

void PacketStreamWriter::Open(const std::string& filename, unsigned int buffer_size_bytes)
{
    // Open file for writing
    buffer.open(filename, buffer_size_bytes);
    
    // Start of file magic
    writer.write(PANGO_MAGIC.c_str(), PANGO_MAGIC.size());

    WritePangoHeader();
}

PacketStreamWriter::~PacketStreamWriter()
{
    WriteStats();
}

PacketStreamSourceId PacketStreamWriter::AddSource(
    const std::string& type,
    const std::string& uri,
    const std::string& json_frame,
    const std::string& json_header,
    const std::string& json_aux_types
) {
    const std::string ns = "";   // type + "::";

    std::string err;

    PacketStreamSource pss;
    picojson::value typed_aux;
    picojson::value typed_frame;

    picojson::parse(pss.header,json_header.begin(), json_header.end(), &err);
    if(!err.empty()) throw std::runtime_error("Frame header parse error: " + err);

    picojson::parse(typed_aux,json_aux_types.begin(), json_aux_types.end(), &err);
    if(!err.empty()) throw std::runtime_error("Frame types parse error: " + err);

    picojson::parse(typed_frame,json_frame.begin(), json_frame.end(), &err);
    if(!err.empty()) throw std::runtime_error("Frame definition parse error: " + err);

    if( typed_aux.is<picojson::object>() ) {
        typemap.AddTypes(ns, typed_aux.get<picojson::object>() );
        PacketStreamTypeId frame_id = typemap.CreateOrGetType(ns, typed_frame);
        typemap.AddAlias(ns + PANGO_FRAME, frame_id);

        picojson::value json_src(picojson::object_type,false);
        json_src.get<picojson::object>()[json_hdr_type] = picojson::value(type);
        json_src.get<picojson::object>()[json_hdr_uri] = picojson::value(uri);
        json_src.get<picojson::object>()[json_hdr_header] = pss.header;
        json_src.get<picojson::object>()[json_hdr_typed_aux] = typed_aux;
        json_src.get<picojson::object>()[json_hdr_typed_frame] = typed_frame;

        WriteTag(TAG_ADD_SOURCE);
        json_src.serialize(std::ostream_iterator<char>(writer), true);

        const size_t src_id = sources.size();

        pss.type = type;
        pss.uri = uri;
        pss.frametype = typemap.GetTypeId(ns + PANGO_FRAME);

        sources.push_back( pss );
        return src_id;
    }else{
        throw std::runtime_error("Frame definition must be JSON object.");
    }
}

void PacketStreamWriter::WriteSourceFrame(PacketStreamSourceId src, const char* data, size_t n)
{
    // Write SOURCE_FRAME tag and source id
    WriteTag(TAG_SRC_FRAME);
    WriteTimestamp();
    WriteCompressedUnsignedInt(src);

    // Write frame size if dynamic so it can be skipped over easily
    const PacketStreamType& frametype = typemap.GetType(sources[src].frametype);
    if(!frametype.IsFixedSize()) {
        WriteCompressedUnsignedInt(n);
    }else if(frametype.size_bytes != n) {
        throw std::runtime_error("Attempting to write frame of wrong size");
    }

    // Write data
    writer.write(data, n);
    bytes_written += n;
}

const std::string CurrentTimeStr() {
    time_t time_now = time(0);
    struct tm time_struct = *localtime(&time_now);
    char buffer[80];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %X", &time_struct);
    return buffer;
}

void PacketStreamWriter::WritePangoHeader()
{
    // Write Header
    picojson::value pango(picojson::object_type,false);
    pango["pangolin_version"] = picojson::value(PANGOLIN_VERSION_STRING);
    pango["date_created"] = picojson::value( CurrentTimeStr() );
    pango["endian"] = picojson::value("little_endian");

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

//////////////////////////////////////////////////////////////////////////
// PacketStreamReader
//////////////////////////////////////////////////////////////////////////

PacketStreamReader::PacketStreamReader()
    : next_tag(0), frames(0)
{
}

PacketStreamReader::PacketStreamReader(const std::string& filename, bool realtime)
    : next_tag(0), frames(0)
{
    Open(filename, realtime);
}

void PacketStreamReader::Open(const std::string& filename, bool realtime)
{
    if (reader.is_open()) {
        Close();
    }

    this->realtime = realtime;
    ++playback_devices;

    const size_t PANGO_MAGIC_LEN = PANGO_MAGIC.size();
    char buffer[10];

    reader.open(filename.c_str(), std::ios::in | std::ios::binary);
    if (!reader.good()) {
        throw std::runtime_error("Unable to open file '" + filename + "'.");
    }

    // Check file magic matches expected value
    reader.read(buffer, PANGO_MAGIC_LEN);
    if (!reader.good() || strncmp((char*)buffer, PANGO_MAGIC.c_str(), PANGO_MAGIC_LEN)) {
        throw std::runtime_error("Unrecognised or corrupted file header.");
    }

    ReadTag();

    // Read any source headers, stop on first frame
    while (next_tag == TAG_PANGO_HDR || next_tag == TAG_ADD_SOURCE) {
        ProcessMessage();
    }
}

void PacketStreamReader::Close()
{
    reader.close();
    --playback_devices;

    sources.clear();
    typemap.Clear();
}

PacketStreamReader::~PacketStreamReader()
{    
    Close();
}

bool PacketStreamReader::ReadToSourceFrameAndLock(PacketStreamSourceId src_id)
{
    read_mutex.lock();

    // Walk over data that no-one is interested in
    int nxt_src_id;
    double time_s;

    ProcessMessagesUntilSourceFrame(nxt_src_id, time_s);

    while(nxt_src_id != (int)src_id) {
        if(nxt_src_id == -1) {
            // EOF or something critical
            read_mutex.unlock();
            return false;
        }else{
//            // Wait for another next frame to get read
//            new_frame.wait(read_mutex);
            ReadOverSourceFramePacket(nxt_src_id);
            ReadTag();
        }

        ProcessMessagesUntilSourceFrame(nxt_src_id, time_s);
    }

    // Sync time to start of stream if there are no other playback devices
    if(frames == 0 && playback_devices == 1) {
        ResetLoggingSystemTimeSeconds(time_s);
    }

    if(realtime) {
        // Wait correct amount of time
        const double time_diff = time_s - LoggingSystemTimeSeconds();
        boostd::this_thread::sleep_for(
            boostd::chrono::duration_cast<boostd::chrono::milliseconds>(
                boostd::chrono::duration<double>(time_diff)
            )
        );
    }

    return true;
}

void PacketStreamReader::ReleaseSourceFrameLock(PacketStreamSourceId src_id)
{
    ReadTag();
    ++frames;
    read_mutex.unlock();
}

void PacketStreamReader::ProcessMessage()
{
    // Read one frame / header
    switch (next_tag) {
    case TAG_PANGO_HDR:
        ReadHeaderPacket();
        break;
    case TAG_ADD_SOURCE:
        ReadNewSourcePacket();
        break;
    case TAG_PANGO_STATS:
        ReadStatsPacket();
        break;
    case TAG_SRC_FRAME:
    {
        const size_t src_id = ReadCompressedUnsignedInt();
        if(src_id >= sources.size()) {
            throw std::runtime_error("Invalid Frame Source ID.");
        }
        ReadOverSourceFramePacket(src_id);
        break;
    }
    case TAG_END:
        return;
    default:
        // TODO: Resync
        throw std::runtime_error("Unknown packet type.");
    }
    if(!ReadTag()) {
        // Dummy end tag
        next_tag = TAG_END;
    }}

// return src_id
void PacketStreamReader::ProcessMessagesUntilSourceFrame(int &nxt_src_id, double &time_s)
{
    while(true)
    {
        // Read one frame / header
        switch (next_tag) {
        case TAG_PANGO_HDR:
            ReadHeaderPacket();
            break;
        case TAG_ADD_SOURCE:
            ReadNewSourcePacket();
            break;
        case TAG_PANGO_STATS:
            ReadStatsPacket();
            break;
        case TAG_SRC_FRAME:
        {
            time_s = ReadTimestamp();
            nxt_src_id = ReadCompressedUnsignedInt();
            if(nxt_src_id >= (int)sources.size()) {
                throw std::runtime_error("Invalid Frame Source ID.");
            }
            return;
        }
        case TAG_END:
            nxt_src_id = -1;
            return;
        default:
            // TODO: Resync
            throw std::runtime_error("Unknown packet type.");
        }
        if(!ReadTag()) {
            // Dummy end tag
            next_tag = TAG_END;
        }
    }
}

bool PacketStreamReader::ReadTag()
{
    if(reader.good()) {
        reader.read((char*)&next_tag, TAG_LENGTH );
    }

    return reader.good();
}

void PacketStreamReader::ReadHeaderPacket()
{
    picojson::value json_header;
    picojson::parse(json_header, reader);

    // Consume newline
    char buffer[1];
    reader.read(buffer,1);
}

void PacketStreamReader::ReadNewSourcePacket()
{
    picojson::value json;
    picojson::parse(json, reader);
    reader.get(); // consume newline

    if(!json.contains(json_hdr_type) || !json.contains(json_hdr_uri)) {
        throw std::runtime_error("Missing required fields for source.\n" );
    }

    picojson::value jstype = json[json_hdr_type];
    picojson::value jsuri  = json[json_hdr_uri];

    if( !jstype.is<std::string>() || !jsuri.is<std::string>() ) {
        throw std::runtime_error("Missing required fields for source.\n" );
    }

    PacketStreamSource ps;
    ps.type = jstype.get<std::string>();
    ps.uri  = jsuri.get<std::string>();

    const std::string ns = ""; // ps.type + "::";

    if(json.contains(json_hdr_header)) {
        ps.header = json[json_hdr_header];
    }

    if(json.contains(json_hdr_typed_aux)) {
        picojson::value jsaux = json[json_hdr_typed_aux];
        typemap.AddTypes( ns, jsaux.get<picojson::object>() );
    }

    if(json.contains(json_hdr_typed_frame)) {
        ps.frametype = typemap.CreateOrGetType(ns, json[json_hdr_typed_frame]);
    }

    sources.push_back(ps);
}

void PacketStreamReader::ReadStatsPacket()
{
    picojson::value json;
    picojson::parse(json, reader);
    reader.get(); // consume newline
}

void PacketStreamReader::ReadOverSourceFramePacket(PacketStreamSourceId src_id)
{
    const PacketStreamSource& src = sources[src_id];
    const PacketStreamType& src_type = typemap.GetType(src.frametype);

    if(src_type.IsFixedSize()) {
        reader.ignore(src_type.size_bytes);
    }else{
        size_t size_bytes = ReadCompressedUnsignedInt();
        reader.ignore(size_bytes);
    }
}


}
