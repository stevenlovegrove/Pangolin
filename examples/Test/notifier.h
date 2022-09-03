#include <libfswatch/c++/monitor_factory.hpp>
#include <thread>
#include <functional>

struct FileNotifier
{
    FileNotifier(const std::string& path, const std::function<void()> user_func)
        : user_func(user_func)
    {
        active_monitor.reset(
            fsw::monitor_factory::create_monitor(
                fsw_monitor_type::system_default_monitor_type,
                {path},
                [](const std::vector<fsw::event>& event, void * data){
                    reinterpret_cast<FileNotifier*>(data)->callback(event);
                },
                this
                )
            );

        mon_thread = std::thread([&](){ active_monitor->start(); });
    }

    ~FileNotifier()
    {
        if(active_monitor) active_monitor->stop();
        mon_thread.join();
    }

private:
    void callback(const std::vector<fsw::event>& event)
    {
        user_func();
    }

    std::function<void()> user_func;
    std::unique_ptr<fsw::monitor> active_monitor;
    std::thread mon_thread;
};
