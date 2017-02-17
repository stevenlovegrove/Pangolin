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

#define SCOPED_LOCK lock_guard<decltype(_lock)> lg(_lock)

//#define SCOPED_LOCK

//#define CHK std::cerr << __FILE__ << "::" << __FUNCTION__ << "::" << __LINE__ << std::endl;

namespace pangolin
{

size_t iPacketStream::Stream::readUINT()
{
    size_t n = 0;
    size_t v = get();
    uint32_t shift = 0;
    while (good() && (v & 0x80))
    {
        n |= (v & 0x7F) << shift;
        shift += 7;
        v = get();
    }
    if (!good())
        return static_cast<size_t>(-1);
    return n | (v & 0x7F) << shift;
}

int64_t iPacketStream::Stream::readTimestamp()
{
    int64_t time_us;
    read(reinterpret_cast<char*>(&time_us), sizeof(int64_t));
    return time_us;
}

pangoTagType iPacketStream::Stream::readTag()
{
    auto r = peekTag();
    _tag = 0;
    return r;
}

pangoTagType iPacketStream::Stream::readTag(pangoTagType x)
{
    auto r = readTag();
    if (r != x)
        throw std::runtime_error(("Tag mismatch error: expected tag '" + tagName(r) + "' does not match found tag '" + tagName(x) + "'").c_str());
    return r;
}

pangoTagType iPacketStream::Stream::peekTag()
{
    if (!_tag)
    {
        _tag = 0;
        read(reinterpret_cast<char*>(&_tag), TAG_LENGTH);
        if (!good())
            _tag = TAG_END;
    }
    return _tag;
}

char iPacketStream::Stream::get()
{
    if (_data_len)
        _data_len--;
    _tag = 0;
    return parent::get();
}

size_t iPacketStream::Stream::read(char* target, size_t len)
{
    _tag = 0;
    _frame.src = static_cast<decltype(_frame.src)>(-1);
    parent::read(target, len);
    if (_data_len)
        _data_len = std::max(_data_len - gcount(), 0ul);
    return gcount();
}

size_t iPacketStream::Stream::skip(size_t len)
{
    ignore(len);
    if (_data_len)
        _data_len = std::max(_data_len - len, 0ul);
    _tag = 0;
    _frame.src = static_cast<decltype(_frame.src)>(-1);
    return len;
}

std::streampos iPacketStream::Stream::tellg()
{
    if (!seekable())
        return -1;
    if (_tag)
        return parent::tellg() - std::streamoff(TAG_LENGTH);
    return parent::tellg();
}

void iPacketStream::Stream::seekg(std::streampos target)
{
    if (!seekable())
        return;
    cclear();
    parent::seekg(target);
}

void iPacketStream::Stream::seekg(std::streamoff off, std::ios_base::seekdir way)
{
    if (!seekable())
        return;
    cclear();
    parent::seekg(off, way);
}

static bool valid(pangoTagType t)
{
    switch (t)
    {
    case TAG_PANGO_SYNC:
        case TAG_ADD_SOURCE:
        case TAG_SRC_JSON:
        case TAG_SRC_PACKET:
        case TAG_PANGO_STATS:
        case TAG_PANGO_FOOTER:
        case TAG_END:
        case TAG_PANGO_HDR:
        case TAG_PANGO_MAGIC:
        return true;
    default:
        return false;
    }
}

pangoTagType iPacketStream::Stream::syncToTag() //scan through chars one by one until the last three look like a tag
{
    peekTag();
    char * buffer = reinterpret_cast<char*>(&_tag);

    buffer[3] = 0;

    do
    {
        buffer[0] = buffer[1];
        buffer[1] = buffer[2];
        buffer[2] = get();
    }
    while (good() && !valid(_tag));

    if (!good())
        _tag = TAG_END;

    return _tag;

}

iPacketStream::FrameInfo iPacketStream::Stream::peekFrameHeader(const iPacketStream& p)
{
    if (_frame)
        return _frame;

    auto pos = tellg();

    if (peekTag() == TAG_SRC_JSON)
    {
        readTag(TAG_SRC_JSON);
        _frame.src = readUINT();
        json::parse(_frame.meta, *this);
    }

    readTag(TAG_SRC_PACKET);
    _frame.time = readTimestamp();

    if (_frame)
    {
        if (readUINT() != _frame.src)
            throw std::runtime_error("Frame preceded by metadata for a mismatched source. Stream may be corrupt.");
    }
    else
        _frame.src = readUINT();

    _frame.size = p.sources()[_frame.src].data_size_bytes;
    if (!_frame.size)
        _frame.size = readUINT();
    _frame.sequence_num = p.getPacketIndex(_frame.src);
    _frame.stream_location = pos;

    _tag = TAG_SRC_PACKET;

    return _frame;
}

iPacketStream::FrameInfo iPacketStream::Stream::readFrameHeader(const iPacketStream& p)
{
    auto r = peekFrameHeader(p);
    _frame.src = static_cast<decltype(_frame.src)>(-1);
    _tag = 0;
    return r;
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
    while (_stream.peekTag() == TAG_ADD_SOURCE)
        parseNewSource();
}

void iPacketStream::parseHeader()
{
    _stream.readTag(TAG_PANGO_HDR);

    json::value json_header;
    json::parse(json_header, _stream); //looks like right now, we don't do anything with this.
    _starttime = json_header["time_us"].get<int64_t>();

    if (!_starttime)
        pango_print_warn("Unable to read stream start time. Time sync to treat stream as realtime will not work!\n");

    _stream.get(); // consume newline
}

void iPacketStream::parseNewSource()
{
    _stream.readTag(TAG_ADD_SOURCE);
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

    if (_sources.size() != pss.id)
        throw runtime_error("Id mismatch parsing source descriptors. Possible corrupt stream?");
    _sources.push_back(pss);
    if (_next_packet_framenum.size() < pss.id + 1)
        _next_packet_framenum.resize(pss.id + 1);
    _next_packet_framenum[pss.id] = 0;
}

void iPacketStream::setupIndex()
{
    if (!_stream.seekable())
        return;

    auto pos = _stream.tellg();
    _stream.seekg(-(static_cast<istream::off_type>(sizeof(uint64_t)) + TAG_LENGTH), ios_base::end); //a footer is a tag + index position. //todo: this will choke on trailing whitespace. Make it not choke on trailing whitespace.
    //todo also: this will break if the footer size changes. Make this more dynamic.

    if (_stream.peekTag() == TAG_PANGO_FOOTER)
    {
        _stream.seekg(parseFooter()); //parsing the footer returns the index position
        if (_stream.peekTag() == TAG_PANGO_STATS)
            parseIndex();
    }

    _stream.clear();
    _stream.seekg(pos);
}

streampos iPacketStream::parseFooter() //returns position of index.
{
    _stream.readTag(TAG_PANGO_FOOTER);
    uint64_t index;
    _stream.read(reinterpret_cast<char*>(&index), sizeof(index));
    return index;
}

void iPacketStream::parseIndex()
{
    _stream.readTag(TAG_PANGO_STATS);
    json::value json;
    json::parse(json, _stream);

    if (json.contains("src_packet_index")) //this is a two-dimensional serialized array, [source id][sequence number] ---> packet position in stream
    {
        const auto& json_index = json["src_packet_index"].get<json::array>();  //reference to the whole array
        _index = std::move(packetIndex(json_index));
    }

}

iPacketStream::FrameInfo iPacketStream::_nextFrame()
{
    while (1)
    {
        auto t = _stream.peekTag();

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
            return _stream.peekFrameHeader(*this);
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
    lock(); //we cannot use a scoped lock here, because we may not want to release the lock, depending on what we find.
    try
    {
        while (1)
        {
            auto fi = _nextFrame();
            if (!fi)
            {
                release();
                return fi;
            }
            else //we need to do a few thing with each frame we see.
            {
                ++_next_packet_framenum[fi.src]; //so we have accurate sequence numbers for frames.
                if (_stream.seekable())
                {
                    if (!_index.has(fi.src, fi.sequence_num))
                        _index.add(fi.src, fi.sequence_num, fi.stream_location);  //if it's not in the index for some reason, add it.
                    else if (_index.position(fi.src, fi.sequence_num) != fi.stream_location)
                        pango_print_warn("Stream position does not index position. Index may be corrupt.\n");
                }
                _stream.data_len(fi.size); //now we are positioned on packet data for n characters.
            }

            if (sync)
                waitForTimeSync(*sync, fi.time); //if we are doing timesync, wait, even if it's not our packet.

            if (fi.src == src) //if it's ours, return it and
                return _stream.readFrameHeader(*this); //don't release lock

            _stream.skip(fi.size); //otherwise skip it and get the next one.
        }
    }
    catch (std::exception &e) //since we are not using a scoped lock, we must catch and release.
    {
        release();
        throw e;
    }
    catch (...) //we will always release, even if we cannot identify the exception.
    {
        release();
        throw std::runtime_error("Caught an unknown exception");
    }
}

size_t iPacketStream::readraw(char* target, size_t len)
{
    if (!_stream.data_len())
        throw runtime_error("Packetstream not positioned on data block. nextFrame() should be called before readraw().");
    else if (_stream.data_len() < len)
    {
        pango_print_warn("readraw() requested read of %zu bytes when only %zu bytes remain in data block. Trimming to available data size.", len, _stream.data_len());
        len = _stream.data_len();
    }
    auto r = _stream.read(target, len);
    if (!_stream.data_len()) //we are done reading, and should release the lock from nextFrame()
        release();
    return r;
}

size_t iPacketStream::skip(size_t len)
{
    if (!_stream.data_len())
        throw runtime_error("Packetstream not positioned on data block. nextFrame() should be called before skip().");
    else if (_stream.data_len() < len)
    {
        pango_print_warn("skip() requested skip of %zu bytes when only %zu bytes remain in data block. Trimming to remaining data size.", len, _stream.data_len());
        len = _stream.data_len();
    }
    auto r = _stream.skip(len);
    if (!_stream.data_len()) //we are done skipping, and should release the lock from nextFrame()
        release();
    return r;
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

    if (!_stream.seekable())
        throw std::runtime_error("Stream is not seekable (probably a pipe).");

    if (src > _sources.size())
        throw std::runtime_error("Invalid Frame Source ID.");

    if(_stream.data_len()) //we were in the middle of reading data, and are holding an extra lock. We need to release it, while still holding the scoped lock.
       skip(_stream.data_len());

    while (!_index.has(src, framenum))
    {
        pango_print_warn("seek index miss... reading ahead.\n");

        if (_stream.data_len())
            _stream.skip(_stream.data_len());

        auto fi = nextFrame(src, nullptr);
        if (!fi) //if we hit the end, throw
            throw std::out_of_range("frame number not in sequence");
    }

    auto target_header_start = _index.position(src, framenum);

    _stream.seekg(target_header_start);
    _next_packet_framenum[src] = framenum; //this increments when we parse the header in the next line;
    //THIS WILL BREAK _next_packet_framenum FOR ALL OTHER SOURCES. Todo more refactoring to fix.

    auto r = _stream.peekFrameHeader(*this);  //we need to do this now, because we need r.time in order to sync up our playback.
    if (nullptr != sync && _starttime)
        sync->resyncToOffset(r.time - _starttime); //if we have a sync timer, we need to reset it to play synchronized frame from where we just did a seek to.

    return r;
}

void iPacketStream::SkipSync()
{
    //Assume we have just read PAN, read GO
    if (_stream.get() != 'G' && _stream.get() != 'O')
        throw std::runtime_error("Unknown packet type.");

    while (_stream.peekTag() != TAG_SRC_PACKET && _stream.peekTag() != TAG_END)
        _stream.readTag();
}

size_t iPacketStream::getPacketIndex(PacketStreamSourceId src_id) const //returns the current frame for source
{
    return _next_packet_framenum.at(src_id);
}

}

