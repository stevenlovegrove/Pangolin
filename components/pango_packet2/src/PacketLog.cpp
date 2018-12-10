#include <pangolin/platform.h>
#include <pangolin/utils/file_utils.h>
#include <pangolin/packet2/PacketLog.h>
#include <pangolin/packet2/PacketLogMmapImpl.h>
#include <pangolin/utils/timer.h>

namespace pangolin
{

using Packet = PacketLog::Packet;
using Timepoint = PacketLog::Timepoint;

void TruncateFile(mio::file_handle_type fd, size_t size_bytes)
{
    ftruncate(fd, size_bytes);
}

static inline const std::string CurrentTimeStr()
{
    time_t time_now = time(0);
    struct tm time_struct = *localtime(&time_now);
    char buffer[80];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %X", &time_struct);
    return buffer;
}

inline void writeCompressedUnsignedInt(std::ostream& os, size_t n)
{
    while (n >= 0x80)
    {
    os.put(0x80 | (n & 0x7F));
    n >>= 7;
    }
    os.put(static_cast<unsigned char>(n));
}

inline void writeTimestamp(std::ostream& os, int64_t time_us)
{
    os.write(reinterpret_cast<const char*>(&time_us), sizeof(decltype(time_us)));
}

inline void writeTag(std::ostream& os, const PangoTagType tag)
{
    os.write(reinterpret_cast<const char*>(&tag), TAG_LENGTH);
}


void writeTaggedJson(std::ostream& os, uint32_t tag, const picojson::value& val)
{
    writeTag(os, tag);
    val.serialize(std::ostream_iterator<char>(os), true);
}

picojson::value PangoHeaderJson()
{
    picojson::value pango;
    pango["pangolin_version"] = PANGOLIN_VERSION_STRING;
    pango["time_us"] = Time_us(TimeNow());
    pango["date_created"] = CurrentTimeStr();
    pango["endian"] = "little_endian";
    return pango;
}

InputMmapImpl::InputMmapImpl(const std::string& filename, PacketLog::FileMode /*mode*/)
    : mmap(std::make_shared<mio::ummap_sink>()), end_of_data(nullptr)
{
    constexpr size_t default_initial_size_bytes = 1024*1024*1024; // 1GB

    if(!FileExists(filename)) {
        // Create file.
        std::ofstream f(filename);
    }

    std::error_code err;
    fd = mio::detail::open_file(filename, mio::access_mode::write, err);
    if(err) throw std::runtime_error(err.message());

    size_t file_size_bytes = mio::detail::query_file_size(fd, err);
    if(err) throw std::runtime_error(err.message());

    if(file_size_bytes == 0) {
        // New file - preallocate 'default_initial_size_bytes'
        TruncateFile(fd, default_initial_size_bytes);
        mmap->map(fd, 0, mio::map_entire_file, err);
        if(err) throw std::runtime_error(err.message());
        end_of_data = mmap->data();

        // Write magic
        PANGO_ENSURE(mmap->size() > 5);
        std::memcpy(end_of_data, PANGO_MAGIC.c_str(), PANGO_MAGIC.size());
        end_of_data += PANGO_MAGIC.size();

        // Write file header
        WriteTaggedJson(TAG_PANGO_HDR, PangoHeaderJson());
    }else{
        // Existing file
        mmap->map(fd, 0, mio::map_entire_file, err);
        if(err) throw std::runtime_error(err.message());
        end_of_data = mmap->data() + mmap->size();
    }
}

std::shared_ptr<uint8_t> InputMmapImpl::GetBytes(size_t size_bytes)
{
    if(end_of_data + size_bytes > mmap->end() ) {
        // We don't have enough space
    }
    uint8_t* user_ptr = end_of_data;
    end_of_data += size_bytes;
    return std::shared_ptr<uint8_t>(mmap, user_ptr);
}

void InputMmapImpl::WriteTaggedJson(PangoTagType tag, const picojson::value& val)
{
    const std::string json = val.serialize();
    std::shared_ptr<uint8_t> b = GetBytes(json.size() + sizeof(PangoTagType));
    std::memcpy(b.get(), &tag, sizeof(PangoTagType));
    std::memcpy(b.get() + sizeof(PangoTagType), json.c_str(), json.size());
}

size_t InputMmapImpl::NumSources()
{
    return sources.size();
}

PacketLog::Source& InputMmapImpl::GetSource(size_t i)
{
    return *std::next(sources.begin(), i)->second.get();
}

PacketLog::Source& InputMmapImpl::AddSource()
{
    const auto uuid = uuids::uuid_random_generator()();
    sources[uuid] = std::make_shared<SourceMmapImpl>(shared_from_this(), uuid);
    return *sources[uuid].get();
}

void InputMmapImpl::ForEachPacket(const std::function<void(PacketLog::Packet&)>& func)
{
    for(auto& uuid_src : sources) {
        uuid_src.second->ForEachPacket(func);
    }
}

void InputMmapImpl::ForEachPacket(const std::function<void(const PacketLog::Packet&)>& func) const
{

}

SourceMmapImpl::SourceMmapImpl(const std::shared_ptr<InputMmapImpl>& input, PacketLog::UUID this_uuid)
    : input(input), this_uuid(this_uuid)
{
}

size_t SourceMmapImpl::NumPackets() {}
Packet SourceMmapImpl::GetPacket(size_t idx) {}
Packet SourceMmapImpl::GetPacket(Timepoint capture_time) {}
Packet SourceMmapImpl::AddPacket(Timepoint capture_time, size_t size_t) {}

void SourceMmapImpl::ForEachPacket(const std::function<void(Packet&)>& func) {}
void SourceMmapImpl::ForEachPacket(const std::function<void(const Packet&)>& func) const {}



PacketLog::PacketLog()
{
}

PacketLog::~PacketLog()
{
    Close();
}

void PacketLog::Close()
{
    inputs.clear();
}

PacketLog::Input& PacketLog::OpenFile(const std::string& file, FileMode mode)
{
    auto input = std::make_shared<InputMmapImpl>(file, mode);
    inputs.push_back(input);
    return *input.get();
}

void TestOld()
{
    PacketLog log;
    auto& file = log.OpenFile("test.pango", PacketLog::FileMode::ReadOnly);

    for(size_t i=0; i < file.NumSources(); ++i) {
        auto& src = file.GetSource(i);
    }
}

void TestNew()
{
    PacketLog log;
    auto& file = log.OpenFile("test_new.pango", PacketLog::FileMode::ReadWrite);
    auto& src = file.AddSource();

//    for(int i=0; i < 100; ++i) {
//        std::chrono::steady_clock::time_point capture_time_us;
//        size_t size_bytes = 100;
//        // will throw if capture_time_us < prev capture time
//        // will throw if file mode doesn't support writing / growing
//        PacketLog::Packet p = src.AddPacket(capture_time_us, size_bytes);
//        PANGO_ENSURE(p.size_bytes == 100);
//        PANGO_ENSURE(p.data);
//        std::fill( p.data.get(), p.data.get()+size_bytes, (uint8_t)i);
//    }

//    {
//        PacketLog::Packet p = src.GetPacket(50);
//        PANGO_ENSURE(p.size_bytes == 100);
//        PANGO_ENSURE(p.data);
//        PANGO_ENSURE(p.data.get()[0] == uint8_t(50));
//        std::fill( p.data.get(), p.data.get()+p.size_bytes, (uint8_t)10);
//    }

//    // Iterate over ALL Packets
//    file.ForEachPacket( [](const PacketLog::Packet& p){
//        std::cout << p.capture_time_us.time_since_epoch().count() << std::endl;
//    });

//    // Iterate over packets for source
//    src.ForEachPacket( [](const PacketLog::Packet& p){
//        std::cout << p.capture_time_us.time_since_epoch().count() << std::endl;
//    });
}

}
