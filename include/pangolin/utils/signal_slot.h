#pragma once

#include <functional>
#include <map>

namespace pangolin {

// Based on http://simmesimme.github.io/tutorials/2015/09/20/signal-slot
template <typename... Args>
class Signal
{
public:
    using Id = size_t;

    Signal()
        : current_id_(0)
    {
    }

    Signal(const Signal&) = delete;

    Signal(Signal&&) = default;

    // connects a std::function to the signal. The returned
    // value can be used to disconnect the function again
    Id Connect(const std::function<void(Args...)>& slot) const {
        slots_.insert(std::make_pair(++current_id_, slot));
        return current_id_;
    }

    // disconnects a previously connected function
    void Disconnect(Id id) const {
        slots_.erase(id);
    }

    // disconnects all previously connected functions
    void DisconnectAll() const {
        slots_.clear();
    }

    // calls all connected functions
    void operator()(Args... p) {
        for(auto it : slots_) {
            it.second(p...);
        }
    }

private:
    mutable std::map<Id, std::function<void(Args...)>> slots_;
    mutable Id current_id_;
};

template <typename... Args>
class Connection
{
public:
    using SignalT = Signal<Args...>;

    Connection()
        : signal(nullptr), id(0)
    {
    }

    ~Connection()
    {
        if(signal) {
            signal->Disconnect(id);
        }
    }

    void Connect(SignalT& signal, const std::function<void(Args...)>& slot) {
        this->signal = &signal;
        id = signal.Connect(slot);
    }

private:
    SignalT* signal;
    typename SignalT::Id id;
};

}
