
#pragma once

#include <pangolin/platform.h>
#include <pangolin/utils/picojson.h>
#include <pangolin/utils/file_extension.h>
#include <pangolin/utils/timer.h>
//#include <pangolin/log/packetstream.h>

namespace pangolin
{

using pangoTagType = uint32_t;

const static std::string PANGO_MAGIC = "PANGO";

const unsigned int TAG_LENGTH = 3;

#define PANGO_TAG(a,b,c) ( (c<<16) | (b<<8) | a)
const uint32_t TAG_PANGO_HDR    = PANGO_TAG('L', 'I', 'N');
const uint32_t TAG_PANGO_MAGIC  = PANGO_TAG('P', 'A', 'N');
const uint32_t TAG_PANGO_SYNC   = PANGO_TAG('S', 'Y', 'N');
const uint32_t TAG_PANGO_STATS  = PANGO_TAG('S', 'T', 'A');
const uint32_t TAG_PANGO_FOOTER = PANGO_TAG('F', 'T', 'R');
const uint32_t TAG_ADD_SOURCE   = PANGO_TAG('S', 'R', 'C');
const uint32_t TAG_SRC_JSON     = PANGO_TAG('J', 'S', 'N');
const uint32_t TAG_SRC_PACKET   = PANGO_TAG('P', 'K', 'T');
const uint32_t TAG_END          = PANGO_TAG('E', 'N', 'D');
#undef PANGO_TAG

using PacketStreamSourceId = size_t;

struct PANGOLIN_EXPORT PacketStreamSource
{
    std::string     driver;
    size_t          id;
    std::string     uri;
    json::value     info;
//    json::value     meta;
    int64_t         version;
    int64_t         data_alignment_bytes;
    std::string     data_definitions;
    int64_t         data_size_bytes;

    PacketStreamSource(): id(static_cast<PacketStreamSourceId>(-1)), version(1), data_alignment_bytes(1), data_size_bytes(0){};
    PacketStreamSource(
            const std::string& source_driver,
            const std::string& source_uri,
            const json::value& json_header = json::value(),
            const size_t       packet_size_bytes = 0,
            const std::string& packet_definitions = ""
        ):
            driver(source_driver), id(static_cast<PacketStreamSourceId>(-1)), uri(source_uri), info(json_header), version(1), data_alignment_bytes(1), data_definitions(packet_definitions), data_size_bytes(packet_size_bytes)
    {};
};


#define pss_src_driver "driver"
#define pss_src_id "id"
#define pss_src_info "info"
#define pss_src_uri "uri"
#define pss_src_packet "packet"
#define pss_src_version         "version"
#define pss_pkt_alignment_bytes "alignment_bytes"
#define pss_pkt_definitions "definitions"
#define pss_pkt_size_bytes  "size_bytes"
#define pss_pkt_format_written "format_written"

using sourceIndexType = std::vector<PacketStreamSource>;

inline std::string tagName(int v)
{
    char b[4];
    b[0] = v&0xff;
    b[1] = (v>>8)&0xff;
    b[2] = (v>>16)&0xff;
    b[3] = 0x00;
    return std::string(b);
}


class packetIndex: std::map<PacketStreamSourceId, std::map<size_t, std::streampos>>
{
    public:
    packetIndex(){};
    packetIndex(const json::array& source)
    {
            for (size_t src = 0; src < source.size(); ++src)
            {
                const json::array& row = source[src].get<json::array>();
                for (size_t frame = 0; frame < row.size(); ++frame)
                    add(src, frame, row[frame].get<int64_t>());
            }
    }

    bool has(PacketStreamSourceId source, size_t frame) const {return count(source) ? at(source).count(frame) : false;};
    std::streampos position(PacketStreamSourceId source, size_t frame) const { return at(source).at(frame);};
    size_t packetCount(PacketStreamSourceId source) const {return count(source)? at(source).size() : std::numeric_limits<size_t>::max() ; };

    void add(PacketStreamSourceId source, size_t frame, std::streampos position) {operator[](source)[frame] = position;};
    void add(PacketStreamSourceId source, std::streampos position)
    {
        if (operator[](source).empty())
            add(source, 0, position);
        else
            add(source, operator[](source).rbegin()->first + 1, position);
    }; //rbegin()->first gives us the value of the last key, which will be the highest frame number so far.

    json::array json() const
    {
        json::array index;
        for (const auto& src : *this)
        {
            json::array positions;
            for (const auto& frame : src.second)
            {
                positions.push_back(json::value(frame.second));
            }
            index.push_back(positions);
        }

        return index;
    }


};


}
