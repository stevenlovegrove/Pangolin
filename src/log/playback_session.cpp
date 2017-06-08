#include <pangolin/log/playback_session.h>

namespace pangolin {

PlaybackSession& PlaybackSession::Default()
{
    static PlaybackSession instance;
    return instance;
}

}
