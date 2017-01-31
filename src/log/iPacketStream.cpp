
#include "pangolin/log/iPacketStream.hpp"

using std::string;
using std::istream;
using std::ios;
using std::lock_guard;
using std::runtime_error;
using std::ios_base;
using std::streampos;
using std::streamoff;

#include <thread>

namespace pangolin
{

static inline size_t readCompressedUnsignedInt(std::istream& reader)
{
    size_t n = 0;
    size_t v = reader.get();
    uint32_t shift = 0;
    while (reader.good() && (v & 0x80))
    {
        n |= (v & 0x7F) << shift;
        shift += 7;
        v = reader.get();
    }
    if (!reader.good())
        return static_cast<size_t>(-1);
    return n | (v & 0x7F) << shift;
}

static inline int64_t readTimestamp(std::istream& reader)
{
    int64_t time_us;
    reader.read(reinterpret_cast<char*>(&time_us), sizeof(int64_t));
    return time_us;
}

//#define SCOPED_LOCK lock_guard<decltype(_lock)> lg(_lock)

#define SCOPED_LOCK

pangoTagType iPacketStream::readTag()
{
    SCOPED_LOCK;
    if (!_cachedtag)
    {
        pangoTagType r = 0;
        readraw(reinterpret_cast<char*>(&r), TAG_LENGTH);
        return _stream.good() ? r : TAG_END;
    }
    else
    {
        auto r = _cachedtag;
        _cachedtag = 0;
        return r;
    }
}

pangoTagType iPacketStream::readTag(pangoTagType x)
{
    SCOPED_LOCK;
    if (!_cachedtag)
    {
        pangoTagType r = 0;
        readraw(reinterpret_cast<char*>(&r), TAG_LENGTH);
        if (!_stream.good())
            r = TAG_END;
        else if (r != x)
            throw std::runtime_error(("Tag mismatch error: expected tag '" + tagName(r) + "' does not match found tag '" + tagName(x) + "'").c_str());
        return r;
    }
    else
    {
        auto r = _cachedtag;
        _cachedtag = 0;
        return r;
    }
}

pangoTagType iPacketStream::peekTag()
{
    SCOPED_LOCK;
    if (!_cachedtag)
    {
        _cachedtag = 0;
        readraw(reinterpret_cast<char*>(&_cachedtag), TAG_LENGTH);
        if (!_stream.good())
            _cachedtag = TAG_END;
    }
    return _cachedtag;
}

void iPacketStream::close()
{
    SCOPED_LOCK;
    if (_stream.is_open())
        _stream.close();
    _sources.clear();
    _seekable = false;
    _starttime = 0;
    _cachedtag = 0;
}

void iPacketStream::open(const string& sourcefile)
{
    SCOPED_LOCK;
    close();
    _stream.open(sourcefile.c_str(), std::ios::in | std::ios::binary);
    _seekable = !IsPipe(sourcefile);
    _starttime = 0;
    _cachedtag = 0;
    init();
}

void iPacketStream::init()
{
    SCOPED_LOCK;

    if (!_stream.is_open())
        throw runtime_error("Cannot open stream.");

    for (auto i : PANGO_MAGIC)
    {
        if (_stream.get() != i)
            throw runtime_error("Unrecognised file header.");
        if (!_stream.good())
            throw runtime_error("Bad stream");
    }

    setupIndex();
    parseHeader();
    while (peekTag() == TAG_ADD_SOURCE)
        parseNewSource();
}

void iPacketStream::parseHeader()
{
    SCOPED_LOCK;

    readTag(TAG_PANGO_HDR);

    json::value json_header;
    json::parse(json_header, _stream); //looks like right now, we don't do anything with this.
    _starttime = json_header["time_us"].get<int64_t>();

    if (!_starttime)
        pango_print_warn("Unable to read stream start time. Time sync to treat stream as realtime will not work!\n");

    _stream.get(); // consume newline
}

void iPacketStream::parseNewSource()
{
    SCOPED_LOCK;

    readTag(TAG_ADD_SOURCE);
    json::value json;
    json::parse(json, _stream);
    _stream.get(); // consume newline

    PacketStreamSource pss;
    pss.driver = json[pss_src_driver].get<string>();
    pss.id = json[pss_src_id].get<int64_t>();
    pss.uri = json[pss_src_uri].get<string>();
    pss.info = json[pss_src_info];
    pss.version = json[pss_src_version].get<int64_t>();
    pss.data_alignment_bytes = json[pss_src_packet][pss_pkt_alignment_bytes].get<int64_t>();
    pss.data_definitions = json[pss_src_packet][pss_pkt_definitions].get<string>();
    pss.data_size_bytes = json[pss_src_packet][pss_pkt_size_bytes].get<int64_t>();
//    pss.format_written = Name2ImageFileType(json[pss_src_packet][pss_pkt_format_written].get<string>());

    if (_sources.size() != pss.id)
        throw runtime_error("Id mismatch parsing source descriptors. Possible corrupt stream?");
    _sources.push_back(pss);

    _next_packet_framenum[pss.id] = 0;
}

void iPacketStream::setupIndex()
{
    SCOPED_LOCK;

    if (!_seekable)
        return;

    auto pos = tellg();
    seekg(-(static_cast<istream::off_type>(sizeof(uint64_t)) + TAG_LENGTH), ios_base::end); //a footer is a tag + index position. //todo: this will choke on trailing whitespace. Make it not choke on trailing whitespace.
    //todo also: this will break if the footer size changes. Make this more dynamic.

    if (peekTag() == TAG_PANGO_FOOTER)
    {
        seekg(parseFooter()); //parsing the footer returns the index position
        if (peekTag() == TAG_PANGO_STATS)
            parseIndex();
    }

    _stream.clear();
    seekg(pos);
}

streampos iPacketStream::parseFooter() //returns position of index.
{
    SCOPED_LOCK;

    readTag(TAG_PANGO_FOOTER);
    uint64_t index;
    readraw(reinterpret_cast<char*>(&index), sizeof(index));
    return index;
}

void iPacketStream::parseIndex()
{
    SCOPED_LOCK;

    readTag(TAG_PANGO_STATS);
    json::value json;
    json::parse(json, _stream);

    if (json.contains("src_packet_index")) //this is a two-dimensional serialized array, [source id][sequence number] ---> packet position in stream
    {
        const auto& json_index = json["src_packet_index"].get<json::array>();  //reference to the whole array
        _index = std::move(packetIndex(json_index));
    }

}

//void iPacketStream::parseSourceMeta()
//{
//    SCOPED_LOCK;
//
//    readTag(TAG_SRC_JSON);
//    auto src_id = readCompressedUnsignedInt(_stream);
//
//    if (src_id == static_cast<decltype(src_id)>(-1))
//        return;
//    else if (src_id >= _sources.size())
//        throw std::runtime_error("Invalid Frame Source ID found in metainfo packet.");
//
//    json::parse(_sources[src_id].meta, _stream);
//
//}

iPacketStream::FrameInfo iPacketStream::parsePacketHeader()
{
    SCOPED_LOCK;

    auto pos = tellg(); //on a non-seekable stream, this is probably -1, or some other unusable number, but we won't use it them, so no harm.

    FrameInfo r;

    if (peekTag() == TAG_SRC_JSON)
    {
        readTag(TAG_SRC_JSON);
        r.src = readCompressedUnsignedInt(_stream);
        json::parse(r.meta, _stream);
    }


    readTag(TAG_SRC_PACKET);
    r.time = readTimestamp(_stream);

    if (!r.none())
    {
        if (readCompressedUnsignedInt(_stream) != r.src)
            throw std::runtime_error("Frame preceded by metadata for a mismatched source. Stream may be corrupt.");
    }
    else
        r.src = readCompressedUnsignedInt(_stream);

    r.size = _sources[r.src].data_size_bytes;
    if (!r.size)
        r.size = readCompressedUnsignedInt(_stream);
    r.sequence_num = _next_packet_framenum[r.src];
    _next_packet_framenum[r.src] += 1;

    if (_seekable)
        _index.add(r.src, r.sequence_num, pos);

    return r;
}

iPacketStream::FrameInfo iPacketStream::_nextFrame()
{
    SCOPED_LOCK;

    while (1)
    {
        auto t = peekTag();

        switch (t)
        {
        case TAG_PANGO_SYNC:
            SkipSync();
            break;
        case TAG_ADD_SOURCE:
            parseNewSource();
            break;
        case TAG_SRC_JSON: //frames are sometimes preceded by metadata, but metadata must ALWAYS be followed by a frame from the same source.
        case TAG_SRC_PACKET:
            return parsePacketHeader();
        case TAG_PANGO_STATS:
            parseIndex();
            break;
        case TAG_PANGO_FOOTER: //end of frames
        case TAG_END:
            return FrameInfo(); //none
        case TAG_PANGO_HDR: //shoudln't encounter this
            parseHeader();
            break;
        case TAG_PANGO_MAGIC: //or this
            SkipSync();
            break;
        default: //or anything else
            pango_print_warn("Unexpected packet type: \"%s\". Resyncing()\n", tagName(t).c_str());
            ReSync();
            break;
        }
    }

}

iPacketStream::FrameInfo iPacketStream::nextFrame(PacketStreamSourceId src, SyncTime *sync)
{
    SCOPED_LOCK;

    while (1)
    {
        auto fi = nextFrame(sync);

        if (fi.src == src || fi.none())
            return fi;
//        else if (!_handlers.count(fi.src) || _handlers.at(fi.src) == nullptr)
        skip(fi.size);
    }
}

iPacketStream::FrameInfo iPacketStream::nextFrame(SyncTime *sync)
{
    SCOPED_LOCK;

    auto fi = _nextFrame();

    if (fi.none())
        return fi;

    if (sync)
        waitForTimeSync(*sync, fi.time);

//    if (_handlers.count(fi.src) && _handlers.at(fi.src) != nullptr) //if there is a callback frame handler defined
//    {
//        auto read = _handlers.at(fi.src)(fi, *this); //call it and get the number of characters read
//        if (read < fi.size) //if we didn't read the whole packet, skip the rest.
//            skip(fi.size - read);
//    }

    return fi;
}

size_t iPacketStream::readraw(char* target, size_t len)
{
    SCOPED_LOCK;
    _cachedtag = 0;
    _stream.read(target, len);
    return _stream.gcount();
}

size_t iPacketStream::skip(size_t len)
{
    SCOPED_LOCK;
    _cachedtag = 0;
    _stream.ignore(len);
    return len;
}

void iPacketStream::waitForTimeSync(const SyncTime& timer, int64_t wait_for) const
        {
    if (!_starttime) //if we couldn't read stream time, we cannot sync.
        return;

    timer.waitUntilOffset(wait_for - _starttime);
}

iPacketStream::FrameInfo iPacketStream::seek(PacketStreamSourceId src, size_t framenum, SyncTime *sync)
{
    SCOPED_LOCK;

    if (!_seekable)
        throw std::runtime_error("Stream is not seekable (probably a pipe).");

    if (src > _sources.size())
        throw std::runtime_error("Invalid Frame Source ID.");

    while (!_index.has(src, framenum))
    {
        pango_print_warn("seek index miss... reading ahead.\n");
        auto fi = nextFrame(src, nullptr);
        if (fi.none()) //if we hit the end, throw
            throw std::out_of_range("frame number not in sequence");
        skip(fi.size);
    }


    auto target_header_start = _index.position(src, framenum);

    seekg(target_header_start);
    _next_packet_framenum[src] = framenum; //this increments when we parse the header in the next line;

    auto r = parsePacketHeader();  //we need to do this now, because we need r.time in order to sync up our playback.
    if (nullptr != sync && _starttime)
        sync->resyncToOffset(r.time - _starttime); //if we have a sync timer, we need to reset it to play synchronized frame from where we just did a seek to.

    //this led to a bug at first. Logically, the sequence of commands seems like it would be "seek, read data", but the old interface, on a higher level, requires "seek, grabnext, readdata".
    //So we need to go to the beginning of the frame header, not the beginning of the data.
    seekg(target_header_start);

    //note to future developers. If you are reading this, please put your pitchforks and torches away. Yes, this is a bad interface and needs to be refactored. Yes, _next_packet_framenum will break for any source after a seek
    //on another source. Yes, the caller shouldn't have to care where the underlying stream pointer is, whether it's on a header or on the data afterwards... things should just *work*.

    //But let's just get this working right now. And if you're reading this, that's as far as I got.

    return r;
}

void iPacketStream::SkipSync()
{
    //Assume we have just read PAN, read GO
    _cachedtag = 0;
    if (_stream.get() != 'G' && _stream.get() != 'O')
        throw std::runtime_error("Unknown packet type.");

    while (peekTag() != TAG_SRC_PACKET && peekTag() != TAG_END)
        readTag();
}

void iPacketStream::ReSync() //this appears to sometimes segfault or get return prematurely (not sure which) if we try to resync through lots of frame data.
//should only be used for pipe sync.
//It's by no means impossible for raw binary image data to contain the sequence "PKT" anyway, so we might have to run this repeatedly.
{
    auto curr_tag = peekTag();
    char * buffer = reinterpret_cast<char*>(&curr_tag);

    buffer[3] = 0;

    do
    {
        buffer[0] = buffer[1];
        buffer[1] = buffer[2];
        readraw(&buffer[2], 1);
    }
    while (_stream.good() && curr_tag != TAG_SRC_PACKET);

    if (!_stream.good())
        _cachedtag = TAG_END;
    else
        _cachedtag = curr_tag;

}

size_t iPacketStream::getPacketIndex(PacketStreamSourceId src_id) const //returns the current frame for source
{
    auto it = _next_packet_framenum.find(src_id);
    return it != _next_packet_framenum.end() ? it->second: 0;
}

}

