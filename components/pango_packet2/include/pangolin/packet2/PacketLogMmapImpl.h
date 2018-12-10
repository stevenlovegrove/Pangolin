#pragma once

#include <pangolin/utils/picojson.h>
#include <pangolin/packet2/PacketLog.h>
#include <mio/mmap.hpp>
#include <pangolin/packet2/../../../../pango_packetstream/include/pangolin/log/packetstream_tags.h>

namespace pangolin
{

struct InputBase : public PacketLog::Input
{
    virtual void WriteTaggedJson(uint32_t tag, const picojson::value& val) = 0;
};

struct InputMmapImpl : public InputBase, public std::enable_shared_from_this<InputMmapImpl>
{
    InputMmapImpl(const std::string& filename, PacketLog::FileMode /*mode*/);

    size_t NumSources() override;

    PacketLog::Source& GetSource(size_t i) override;

    PacketLog::Source& AddSource() override;

    void ForEachPacket(const std::function<void(PacketLog::Packet&)>& func) override;

    void ForEachPacket(const std::function<void(const PacketLog::Packet&)>& func) const override;


    void WriteTaggedJson(PangoTagType tag, const picojson::value& val) override;

    // Return ptr whose deleter refers to mmap.
    std::shared_ptr<uint8_t> GetBytes(size_t size_bytes);
private:
    mio::file_handle_type fd;
    std::map<PacketLog::UUID,std::shared_ptr<PacketLog::Source>> sources;
    std::shared_ptr<mio::ummap_sink> mmap;
    uint8_t* end_of_data;
};

struct SourceMmapImpl : public PacketLog::Source
{
    using Packet = PacketLog::Packet;
    using Timepoint = PacketLog::Timepoint;

    SourceMmapImpl(const std::shared_ptr<InputMmapImpl>& input, PacketLog::UUID this_uuid);

    size_t NumPackets() override;
    Packet GetPacket(size_t idx) override;
    Packet GetPacket(Timepoint capture_time) override;
    Packet AddPacket(Timepoint capture_time, size_t size_t) override;

    void ForEachPacket(const std::function<void(Packet&)>& func) override;
    void ForEachPacket(const std::function<void(const Packet&)>& func) const override;

private:
    std::weak_ptr<InputMmapImpl> input;

    // unique id for this sources session of packets
    PacketLog::UUID this_uuid;

    // If this is derived data from another source, express this here.
    PacketLog::UUID from_uuid;

    // Driver and serial should uniquely describe device
    std::string     driver;
    std::string     serial;

    size_t          id;
    std::string     uri;
//        picojson::value info;
    int64_t         version;
    int64_t         data_alignment_bytes;
    std::string     data_definitions;
    int64_t         data_size_bytes;
};

}
