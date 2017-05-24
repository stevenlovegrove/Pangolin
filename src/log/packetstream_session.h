#pragma once

#include <map>
#include <string>
#include <memory>
#include <functional>

#include <pangolin/log/packetstream_reader.h>

namespace pangolin {

struct PlaybackSession
{
};

template<typename T = size_t>
class Token
{
public:
    using TokenType = T;
    using TokenFunc = std::function<void(const TokenType&)>;

    Token(TokenType token, TokenFunc unregister)
        : token(token), unregister(unregister)
    {

    }

    // No copy constructor
    Token(const Token&) = delete;

    // Default move constructor
    Token(Token&&) = default;

    ~Token()
    {
        if(unregister) {
            unregister(token);
        }
    }

    const TokenType& get()
    {
        return token;
    }

private:
    TokenType token;
    TokenFunc unregister;
};

void ExampleUserUsage()
{
    std::vector<VideoStream> streams = FindVideoStreams(uri);

}

void ExampleDriver()
{
    using StreamToken = std::pair<PacketStreamReader,size_t>;

//    PacketStreamReader& r = OpenPacketStream(filename);

    std::string filename;

    PlaybackSession session;

    Token driver_token = session.RegisterForFileSource(filename,
        [](const PacketStreamSource& src){
            return (src.driver == "test");
        }
    );

    session.WaitFor(driver_token);



}

class PacketstreamSession
{
public:


protected:
    std::map<std::string,std::unique_ptr<PacketStreamReader>> readers;
};

}
