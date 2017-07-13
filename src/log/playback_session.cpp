#include <pangolin/log/playback_session.h>

namespace pangolin {

std::shared_ptr<PlaybackSession> PlaybackSession::Default()
{
    static std::shared_ptr<PlaybackSession> instance = std::make_shared<PlaybackSession>();
    return instance;
}

}
