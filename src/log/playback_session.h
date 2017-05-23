#pragma once

#include <memory>
#include <vector>

#include <pangolin/log/packetstream_reader.h>
#include <pangolin/utils/file_utils.h>

namespace pangolin {

class PlaybackSession
{
public:
    // Singleton Instance
    static PlaybackSession& Default();

    // Return thread-safe, shared instance of PacketStreamReader, providing
    // serialised read for PacketStreamReader
    std::shared_ptr<PacketStreamReader> Open(const std::string& filename)
    {
        const std::string path = SanitizePath(PathExpand(filename));

        auto i = readers.find(path);
        if(i == readers.end()) {
            auto psr = std::make_shared<PacketStreamReader>(path);
            readers[path] = psr;
            return psr;
        }else{
            return i->second;
        }
    }

    int64_t PlaybackTime_us() const
    {
        return time
    }


private:
    std::map<std::string,std::shared_ptr<PacketStreamReader>> readers;
    SyncTime time;
};

}
