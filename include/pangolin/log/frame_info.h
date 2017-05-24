#pragma once

#include <pangolin/log/packetstream_source.h>

namespace pangolin {

// Contains information about the queued-up frame. Relies on RVO.
// Obtained from nextFrame() function
struct FrameInfo
{
    FrameInfo()
        : src(static_cast<decltype(src)>(-1)), time(-1),
          size(static_cast<decltype(size)>(-1)),
          sequence_num(static_cast<decltype(sequence_num)>(-1))
    {
    }

    bool None() const
    {
        return src == static_cast<decltype(src)>(-1);
    }

    operator bool() const
    {
        return !None();
    }

    PacketStreamSourceId src;
    int64_t time;
    size_t size;
    size_t sequence_num;
    picojson::value meta;

    // The 'frame' includes the json and the packet.
    std::streampos frame_streampos;
    std::streampos packet_streampos;
};

}
