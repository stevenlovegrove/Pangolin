#pragma once

#include <libfswatch/c++/monitor_factory.hpp>
#include <thread>
#include <functional>

struct FileNotifier
{
    FileNotifier(const std::vector<std::string>& paths, const std::function<void()> user_func)
        : user_func(user_func), paths(paths)
    {
        mon_thread = std::thread([this](){ monitor_thread(); });
    }

    FileNotifier(const std::function<void()> user_func)
        : FileNotifier({}, user_func)
    {
    }

    void AddPaths(const std::vector<std::string>& new_paths)
    {
        paths.insert(paths.end(), new_paths.begin(), new_paths.end());

        if(active_monitor) {
            // stopping the monitor will cause it to get
            // restarted with new paths
            active_monitor->stop();
        }
    }

    void ClearPaths()
    {
        paths.clear();
    }

    ~FileNotifier()
    {
        stop_and_wait();
    }

private:
    void stop_and_wait()
    {
        should_run = false;
        if(active_monitor)
            active_monitor->stop();
        mon_thread.join();
    }

    void monitor_thread()
    {
        should_run = true;

        while(should_run) {
            // Update path spec
            active_monitor.reset(
                fsw::monitor_factory::create_monitor(
                    fsw_monitor_type::system_default_monitor_type,
                    paths,
                    [](const std::vector<fsw::event>& event, void * data){
                        reinterpret_cast<FileNotifier*>(data)->callback(event);
                    },
                    this
                    )
                );

            // blocks
            active_monitor->start();
        }
    }

    void callback(const std::vector<fsw::event>& event)
    {
        user_func();
    }

    std::function<void()> user_func;
    std::unique_ptr<fsw::monitor> active_monitor;
    std::thread mon_thread;
    std::vector<std::string> paths;
    bool should_run;
};
