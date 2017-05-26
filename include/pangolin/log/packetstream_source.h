#pragma once

#include <pangolin/platform.h>
#include <pangolin/utils/picojson.h>
#include <iostream>

namespace pangolin {

using PacketStreamSourceId = size_t;

struct PANGOLIN_EXPORT PacketStreamSource
{
    struct PacketInfo
    {
        std::streampos pos;
        int64_t capture_time;
    };

    PacketStreamSource()
        : id(static_cast<PacketStreamSourceId>(-1)), version(1),
          data_alignment_bytes(1), data_size_bytes(0),
          next_packet_id(0)
    {
    }

    PacketStreamSource(
            const std::string& source_driver,
            const std::string& source_uri,
            const picojson::value& json_header = picojson::value(),
            const size_t       packet_size_bytes = 0,
            const std::string& packet_definitions = ""
    ) : driver(source_driver), id(static_cast<PacketStreamSourceId>(-1)),
        uri(source_uri), info(json_header), version(1),
        data_alignment_bytes(1), data_definitions(packet_definitions),
        data_size_bytes(packet_size_bytes),
        next_packet_id(0)
    {
    }

    std::streampos FindSeekLocation(size_t packet_id)
    {
        if(packet_id < index.size()) {
            return index[packet_id].pos;
        }else{
            return std::streampos(-1);
        }

    }

    std::string     driver;
    size_t          id;
    std::string     uri;
    picojson::value info;
    int64_t         version;
    int64_t         data_alignment_bytes;
    std::string     data_definitions;
    int64_t         data_size_bytes;

    size_t          next_packet_id;
    std::vector<PacketInfo> index;
};

}
