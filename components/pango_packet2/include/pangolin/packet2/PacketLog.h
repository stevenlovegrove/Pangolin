#pragma once

#include <fstream>
#include <vector>
#include <map>
#include <memory>
#include <uuid.h>
#include <chrono>
#include <functional>

namespace pangolin
{

// A packetlog represents a time-ordered sequence of data from N sources across potentially multiple .pango files / streams
// A 'packet' is an atomic piece of data from one source.
// A 'source' is a packet generator of uniform type
class PacketLog
{
public:
    using Timepoint = std::chrono::steady_clock::time_point;
    using UUID = uuids::uuid;

    enum class FileMode
    {
        StreamingRead,
        ReadOnly,
        WriteOnly,
        ReadWrite,
        ReadWriteFixedSize
    };

    // Represents one data blob from a Source
    struct Packet
    {
        Timepoint capture_time_us;
        std::shared_ptr<uint8_t> data;
        size_t size_bytes;
    };

    // Represents one logical source of data inside an Input
    struct Source
    {
        virtual size_t NumPackets() = 0;
        virtual Packet GetPacket(size_t idx) = 0;
        virtual Packet GetPacket(Timepoint capture_time) = 0;
        virtual Packet AddPacket(Timepoint capture_time, size_t size_t) = 0;

        virtual void ForEachPacket(const std::function<void(Packet&)>& func) = 0;
        virtual void ForEachPacket(const std::function<void(const Packet&)>& func) const = 0;
    };

    // Represents one file / stream
    struct Input
    {
        virtual size_t NumSources() = 0;
        virtual Source& GetSource(size_t i) = 0;
        virtual Source& AddSource() = 0;

        virtual void ForEachPacket(const std::function<void(Packet&)>& func)= 0;
        virtual void ForEachPacket(const std::function<void(const Packet&)>& func) const = 0;
    };

    PacketLog();
    ~PacketLog();

    void Close();
    Input& OpenFile(const std::string& file, FileMode mode);

private:
    std::vector<std::shared_ptr<Input>> inputs;
};

void TestOld();
void TestNew();

}
