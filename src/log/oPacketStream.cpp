
#include "pangolin/utils/file_utils.h"

#include "pangolin/log/oPacketStream.hpp"

using std::ios;

namespace pangolin
{

static inline void writeCompressedUnsignedInt(std::ostream& writer, size_t n)
{
    while (n >= 0x80)
    {
	writer.put(0x80 | (n & 0x7F));
	n >>= 7;
    }
    writer.put(static_cast<unsigned char>(n));
}

static inline const std::string CurrentTimeStr()
{
    time_t time_now = time(0);
    struct tm time_struct = *localtime(&time_now);
    char buffer[80];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %X", &time_struct);
    return buffer;
}

static inline void writeTimestamp(std::ostream& writer)
{
    const auto time_us = Time_us(TimeNow());
    writer.write(reinterpret_cast<const char*>(&time_us), sizeof(decltype(time_us)));
}

static inline void writeTag(std::ostream& writer, const pangoTagType tag)
{
    writer.write(reinterpret_cast<const char*>(&tag), TAG_LENGTH);
}

void oPacketStream::writeHeader()
{
    _stream.write(PANGO_MAGIC.c_str(), PANGO_MAGIC.size());
    json::value pango;
    pango["pangolin_version"] = PANGOLIN_VERSION_STRING;
    pango["time_us"] = Time_us(TimeNow());
    pango["date_created"] = CurrentTimeStr();
    pango["endian"] = "little_endian";

    writeTag(_stream, TAG_PANGO_HDR);
    pango.serialize(std::ostream_iterator<char>(_stream), true);

    for (const auto& source : _sources)
        write(source.id, source);
}

void oPacketStream::write(PacketStreamSourceId id, const PacketStreamSource& source)
{
    json::value serialize;
    serialize[pss_src_driver] = source.driver;
    serialize[pss_src_id] = id;
    serialize[pss_src_uri] = source.uri;
    serialize[pss_src_info] = source.info;
    serialize[pss_src_version] = source.version;
    serialize[pss_src_packet][pss_pkt_alignment_bytes] = source.data_alignment_bytes;
    serialize[pss_src_packet][pss_pkt_definitions] = source.data_definitions;
    serialize[pss_src_packet][pss_pkt_size_bytes] = source.data_size_bytes;

    writeTag(_stream, TAG_ADD_SOURCE);
    serialize.serialize(std::ostream_iterator<char>(_stream), true);
}


PacketStreamSourceId oPacketStream::addSource(PacketStreamSource& source)
{
    source.id = addSource(const_cast<const PacketStreamSource&>(source));
    return source.id;
}

PacketStreamSourceId oPacketStream::addSource(const PacketStreamSource& source)
{
    PacketStreamSourceId r = _sources.size(); //source id is by vector position, so we must reassign.
    _sources.push_back(source);

    if (_open) //we might be a pipe, in which case we may not be open
        write(r, source);

    return r;
}

void oPacketStream::writeMeta(PacketStreamSourceId src, const json::value& data)
{
    writeTag(_stream, TAG_SRC_JSON);
    writeCompressedUnsignedInt(_stream, src);
    data.serialize(std::ostream_iterator<char>(_stream), false);
}

void oPacketStream::writePacket(PacketStreamSourceId src, const char* source, size_t sourcelen, const json::value& meta)
{
    if (_indexable)
        _index.add(src, _stream.tellp()); //record position for seek index

    if (!meta.is<json::null>())
        writeMeta(src, meta);

    writeTag(_stream, TAG_SRC_PACKET);
    writeTimestamp(_stream);
    writeCompressedUnsignedInt(_stream, src);

    if (_sources[src].data_size_bytes)
    {
        if (sourcelen != static_cast<size_t>(_sources[src].data_size_bytes))
            throw std::runtime_error("oPacketStream::writePacket --> Tried to write a fixed-size packet with bad size.");
//	if (compressed)
//	   todo something;
//	else
        _stream.write(source, sourcelen);
        _bytes_written += sourcelen;
    }
    else
    {
        writeCompressedUnsignedInt(_stream, sourcelen);

        //todo handle compressed
        _stream.write(source, sourcelen);
        _bytes_written += sourcelen;
    }
}

void oPacketStream::writeSync()
{
    for (unsigned i = 0; i < 10; ++i)
	writeTag(_stream, TAG_PANGO_SYNC);
}

void oPacketStream::writeEnd()
{
    if (!_indexable)
        return;

    auto indexpos = _stream.tellp();

    writeTag(_stream, TAG_PANGO_STATS);
    json::value stat;
    stat["num_sources"] = _sources.size();
    stat["bytes_written"] = _bytes_written;
    stat["src_packet_index"] = _index.json();
    stat.serialize(std::ostream_iterator<char>(_stream), false);
    writeTag(_stream, TAG_PANGO_FOOTER);
    _stream.write(reinterpret_cast<char*>(&indexpos), sizeof(uint64_t));
}


}


