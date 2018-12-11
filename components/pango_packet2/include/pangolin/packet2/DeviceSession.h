#include <memory>
#include <functional>
#include <vector>

struct FactoryInterface
{
    virtual std::string GetDeviceInterfaceType() const = 0;
    virtual std::string GetHelpMessage() const = 0;
};

template<typename T>
struct TypedFactoryInterface : public FactoryInterface
{
    std::string GetDeviceInterfaceType() const override final
    {
        return typeid(T).name();
    }

};

struct DeviceUID
{
    std::string category;
    std::string serial;
};

struct DeviceInterface
{
    virtual DeviceUID GetDeviceUID() const = 0;
};

struct VideoInterface : public DeviceInterface
{

};

class DeviceSession
{
public:
    // For e.g.
    // Cameras, Depth Cameras, LIDARS, IMU's,
    DeviceSession();

    template<typename DeviceInterface>
    std::shared_ptr<DeviceInterface> Get(const SerialNumber& sn);

    void ForEachDevice(const std::function<void(DeviceInterface&)>& func);
    void ForEachDevice(const std::function<void(const DeviceInterface&)>& func) const;

private:

};



void example(argc, argv)
{
    using namespace pangolin;

    // A sensor session 'opens' all devices (live or playback)
    // If in playback mode, it maintains the playback time.
    // Sensors can be loaded through configuration drivers
    // Playback may be backed by different file formats.
    DeviceSession session(SessionConfig(argc,argv));

    // ... Get (already opened) IMU's, Videos etc through some specification (such as serial number or enumeration)
    std::shared_ptr<VideoInterface> video = session->Get<VideoInterface>(SerialNumber("030100"));

    Signal<VideeoFrameReceived> signal = video.OnFrameReceived.Connect();

    video->OnFrameReceived = [](){
        // ...
    };

    session.Start();
}
