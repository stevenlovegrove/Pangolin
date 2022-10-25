#include <pangolin/log/playback_session.h>

namespace pangolin {

std::shared_ptr<PlaybackSession> PlaybackSession::Default()
{
    static std::shared_ptr<PlaybackSession> instance = std::make_shared<PlaybackSession>();
    return instance;
}

std::shared_ptr<PlaybackSession> PlaybackSession::ChooseFromParams(const ParamReader& reader)
{
    return Choose(reader.Get<bool>("OrderedPlayback"));
}

std::shared_ptr<PlaybackSession> PlaybackSession::ChooseFromParams(const Params& params)
{
    return Choose(params.Get<bool>("OrderedPlayback", false));
}

std::shared_ptr<PlaybackSession> PlaybackSession::Choose(bool use_ordered_playback)
{
    if(use_ordered_playback)
    {
        return Default();
    }
    else
    {
        return std::make_shared<PlaybackSession>();
    }
}
}
