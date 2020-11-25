# Signals

For design choices see [this blog post](https://dreamdota.com/c-17-signals/)

## Why Yet Another Signal-Slot Library?
This library is optimized for video games (and probably other low-latency applications as well). Interestingly, even though the observer pattern is generally useful, it has never been standardized in C++, which leads to the never-ending attempts at improvements by curious people. Many signal-slot libraries do not focus on performance, e.g. `boost::signals2` [invocation can be 90x more expensive than a simple function call](https://stackoverflow.com/questions/22416860/is-boostsignals2-overkill-for-simple-applications).

There are many similar libraries - such as [jl_signal](http://hoyvinglavin.com/2012/08/06/jl_signal/), [nuclex signal/slots](http://blog.nuclex-games.com/2019/10/nuclex-signal-slot-benchmarks/) and [several dozens more](https://github.com/NoAvailableAlias/signal-slot-benchmarks). My work is based on a previous [research](https://github.com/TheWisp/ImpossiblyFastEventCPP17) which focused on the syntax and performance improvements brought by a C++17 feature - `template<auto>`. This library is a combination of modern C++ exploration, system programming and data-structure design. It aims to become feature-complete like `boost::signals`, yet extremely light-weight - both run time and memory footprint - in order to replace _interface_ or `std::function` based callbacks.

`signal` emission is **faster** than virtual function calls. Compared to virtual calls, `signal` calls only take between 22% and 77% of the time, depending on the number and the level of randomness of classes and objects.

## Design Choices

### Direct (Blocking) Calls
In game systems, the logic flow often consists of many fast and weakly ordered function calls. 
Asynchronous calls are rather the exceptions than the default. Thread-safe calls add additional costs, thus should be the exceptions rather than the default.

### Optimized for Emission
Latency is the bottleneck.

### O(1) Connection and Disconnection
In a dynamic world, slots (receivers) are often frequently created and destroyed. A linear search removal algorithm can easily become the performance bottleneck, especially when a large number of slots all disconnect at the same time. Removing by swapping with the end mitigates the problem, but the overall time spent removing N slots with a linear search would still be O(N^2). In this library, a slot is removed by marking its index unused, which then gets skipped and cleaned up in the next emission. Benchmarks have shown that the overhead is dominated by memory accessing (cache misses), rather than checking for null (pipeline stalling).

### Safe Recursion and Modification While Iterating
Just like direct function calls, recursions can naturally emerge from complex and dynamic behaviors. Furthermore, the signals and slots may be side-effected by their own results!

## Usage
Simply include the single header, `signals.hpp`.
A C++17 compliant compiler is necessary.
Give it a try on [Godbolt](https://godbolt.org/z/_N2_5P)!

### Basics
The following example demonstrates how to define, connect and emit a signal.

```cpp
// A function callback
void on_update(float delta) { }

// A member function callback
class my_class{
  void on_update(float delta) { }
};

int main()
{
  // A signal specifying its signature in the template parameter
  fteng::signal<void(float delta)> update;

  // Connects to a function callback
  update.connect(on_update);

  // Connects to an object's member function
  my_class* my_obj = new my_class;
  update.connect<&my_class::on_update>(my_obj);

  // Connects to a lambda callback
  update.connect([](float delta) { });

  // Connects to a generic lambda callback
  update.connect([](auto&&... as) { });

  // Emits the signal
  update(3.14f);

  delete my_obj;
}
```

Signals automatically disconnect from their slots (receivers) upon destruction.
```cpp
class button{
  public: fteng::signal<void(button& btn, bool down)> pressed;
};

class my_special_frame {
  std::vector<button> buttons;

  my_special_frame() {
    buttons.emplace_back();
    buttons.back().pressed.connect<&my_special_frame::on_button_pressed>(this);
  }

  void on_button_pressed(button& btn, bool down) {
    /* ... */
  }
};
```

### Connection Management
Slots don't automatically disconnect from the signal when they go out of scope. 
This is due to the non-intrusive design and the "pay only for what you use" principle.

To help automatically disconnect the slot, the `connect()` method returns an unmanaged (raw) connection, which may be converted to a `fteng::connection` representing the unique ownership.
It is recommended to save this connection into the slot's structure in order to automatically disconnect from the signal in an RAII fashion.

The following design would automatically disconnect the object from the signal when it is deleted.

```cpp
class game { /*...*/ };
fteng::signal<void(const game& instance)> game_created;

class subsystem
{
  //Connects a signal with a lambda capturing 'this'
  fteng::connection on_game_created = game_created.connect([this](const game& instance)
  {
    std::cout << "Game is created.\n";
  });
};

int main()
{
  subsystem* sys1 = new subsystem;

  game game_instance;
  game_created(game_instance); // Notifies each subsystem

  delete sys1; // Automatically disconnects from the signal

  game game_instance2;
  game_created(game_instance2); // Notifies each subsystem. Should not crash.
}
```

Alternatively, you may use a member function for callback.

```cpp
class subsystem
{
  //Connects a signal with a member function
  fteng::connection on_game_created = game_created.connect<&subsystem::on_game_created_method>(this);

  void on_game_created_method(const game& instance)
  {
    std::cout << "Game is created.\n";
  };
};
```

A few important notes about the `connection` object:
- `connection` is default-constructible, moveable but not copyable.
- Destroying the `connection` object would automatically disconnect the associated signal and slot.
- If you know the slot outlives the signal, it's fine to connect them without saving the connection object. There won't be any memory leak.
- If the signal can outlive the slots, store the `connection` in the slot's structure so that it disconnects the signal automatically.

### Connecting / Disconnecting Slots from Callback
Sometimes during the callback, we might want to disconnect the slot from the signal. There are also cases where we want to create or destroy other objects, who just happen to observe the same signal that triggered the callback. The following example demonstrates how these usage are supported by the library.
```cpp
fteng::signal<void(entity eid)> entity_created;

class A
{
  std::unique_ptr<B> b;

  fteng::connection on_entity_created = entity_created.connect([this](entity eid)
  {
    // Creates a 'B' which also connects to the signal.
    // It's fine to connect more objects to the signal during the callback, 
    // With a caveat that they won't be notified this time (but next time).
    b = std::make_unique<B>(); 
  });
};

class B
{
  // C is some class that also listens to entity_created
  std::vector<C*> cs;

  fteng::connection on_entity_created = entity_created.connect([this](entity eid)
  {
    /* ... */
    if (eid == some_known_eid){

      // Imagine this operation automatically disconnects all C objects from entity_created
      // It's fine to disconnect any object from the signal during the callback, no matter if it's
      // the object being called back or any other object. The disconnected objects are skipped over.
      for (C* c : cs) 
        delete c;

      // Also fine
      on_entity_created.disconnect();

      // Also fine - Don't do this in modern C++ though ...
      delete this;
    }
  });
};
```

### Blocking a Connection
A connection can be temporarily disabled with `block()`, so that it won't be notified by the signal until it has been `unblock()` ed again.
```cpp
fteng::signal<void()> sig;

class Foo
{
  fteng::connection conn = sig.connect([this](){
    conn.block();
    sig(); // Now this won't cause an infinite recursion.
    conn.unblock();
  });
};
```

## Performance Benchmark
With a little help of template metaprogramming, I've generated classes of different virtual tables (even though small vtable with just 2 methods).

The bottleneck of emission is the cache loading, therefore it makes sense to test different scenarios depending on object memory addresses and class vtable addresses.
If the objects being called are nicely aligned in the memory, we could expect a speed-up from the cache coherence. Similarly, if all objects are from the same class,
their virtual methods would be the same and therefore a speed-up. In the benchmark, I've tested 4 scenarios where each creates 100,000 objects from at most 100 different classes:
- SAME class, SEQUENTIAL objects: all objects are instances of the same class, and are contiguous in the memory.
- SAME class, RANDOM objects: all objects are instancess of the same class, but are randomly scattered in the memory.
- RANDOM class, SEQUENTIAL objects: each object's class is one of 100 possible classes, but they are contiguous in the memory.
- RANDOM class, RANDOM objects: each object's class is one of 100 possible classes, and they are randomly scattered in the memory.

Xeon E3-1275 V2 @ 3.90 GHz 16.0 GB RAM

|                                  | Signal (member func) | Signal (lambda) | Virtual Call | Sig(mem func) % of Virtual | Sig(lambda) % of Virtual |
| -------------------------------- | -------------------- | --------------- | ------------ | -------------------------- | ------------------------ |
| SAME class, SEQUENTIAL objects   | 303 us               | 335 us          | 482 us       | 62%                        | 69%                      |
| SAME class, RANDOM objects       | 307 us               | 354 us          | 1336 us      | 22%                        | 26%                      |
| RANDOM class, SEQUENTIAL objects | 990 us               | 995 us          | 1288 us      | 76%                        | 77%                      |
| RANDOM class, RANDOM objects     | 998 us               | 1009 us         | 1795 us      | 55%                        | 56%                      |

## Signal-Slots Benchmark Compared to Other Libraries
See https://github.com/NoAvailableAlias/signal-slot-benchmarks
Using Windows + MSVC setup provided by the benchmark (/O2)
Xeon E3-1275 V2 @ 3.90 GHz 16.0 GB RAM

Disclaimer: The benchmark does not represent real-world use cases and the relative rankings are not good indicators of performance. Nevertheless, it does show the `emit` performance of this library is among the top ones while also securing the recursion- and modification-safety. 


| Library | [constr] | [destr] | conn | disconn | reconn | emit | all | threaded | score |
|---------|----------|---------|------|---------|--------|------|-----|----------|-------|
| FTeng Signals | 16033 | 17740 | 7381 | 304829 | 12718 | 107016 | 730 | 0 | 432673 |
| jeffomatic jl_signal | 20778 | 14887 | 77133 | 29369 | 77766 | 109911 | 17880 | 0 | 312059 |
| Nuclex Events | 46275 | 44721 | 18807 | 17958 | 18763 | 109255 | 8477 | 0 | 173259 |
| Wink-Signals | 42809 | 53225 | 9786 | 18696 | 21041 | 96149 | 9590 | 0 | 155262 |
| Montellese cpp-signal | 42558 | 13554 | 11351 | 11600 | 12407 | 108239 | 5597 | 0 | 149194 |
| Ansoulom cpp-observe | 37842 | 20341 | 7011 | 14945 | 11190 | 107264 | 5922 | 0 | 146332 |
| palacaze sigslot | 32652 | 15235 | 9514 | 13086 | 17031 | 97824 | 7219 | 0 | 144674 |
| Yassi | 40914 | 25610 | 7819 | 12252 | 16473 | 96715 | 6773 | 0 | 140032 |
| nano-signal-slot st | 38215 | 18256 | 8877 | 12698 | 14650 | 96543 | 6744 | 0 | 139512 |
| mwthinker Signal | 3556 | 4141 | 8431 | 9887 | 7674 | 108291 | 4563 | 0 | 138845 |
| fr00b0 nod | 21444 | 12154 | 11978 | 28861 | 39660 | 42886 | 12828 | 0 | 136214 |
| * nano-signal-slot ts | 34028 | 11758 | 7223 | 11938 | 13659 | 96010 | 5968 | 699 | 135497 |
| vdksoft signals | 4134 | 5311 | 10221 | 12905 | 11024 | 93769 | 5497 | 0 | 133416 |
| amc522 Signal11 | 19416 | 20536 | 7359 | 11054 | 7871 | 102716 | 4288 | 0 | 133288 |
| SimpleSignal | 3527 | 4407 | 7328 | 7012 | 7088 | 105191 | 3497 | 0 | 130116 |
| pbhogan Signals | 3516 | 4962 | 10071 | 8347 | 10381 | 91944 | 4402 | 0 | 125144 |
| joanrieu signal11 | 1881 | 2477 | 10355 | 13391 | 10598 | 84307 | 5288 | 0 | 123940 |
| * Montellese cpp-signal | 12507 | 4525 | 7500 | 6299 | 7085 | 90935 | 2979 | 466 | 115264 |
| supergrover sigslot | 1981 | 2723 | 3803 | 3834 | 3353 | 96302 | 1730 | 0 | 109022 |
| * palacaze sigslot | 3634 | 2739 | 7378 | 10098 | 11548 | 72604 | 5173 | 625 | 107426 |
| * cpp11nullptr lsignal | 1696 | 1702 | 3505 | 6418 | 3527 | 87205 | 2233 | 358 | 103247 |
| * fr00b0 nod | 11969 | 9026 | 9215 | 17567 | 21581 | 38875 | 7712 | 498 | 95448 |
| nano-signal-slot sts | 42074 | 22216 | 8164 | 13002 | 14266 | 53808 | 5208 | 0 | 94446 |
| * CppFakeIt FastSignals | 3021 | 3477 | 5733 | 11957 | 13855 | 35967 | 5257 | 147 | 72915 |
| * Kosta signals-cpp | 12738 | 4490 | 2652 | 33577 | 2250 | 28149 | 1949 | 240 | 68817 |
| EvilTwin Observer | 2301 | 3587 | 3526 | 6502 | 4453 | 51396 | 2561 | 0 | 68437 |
| * Boost Signals2 | 199 | 385 | 2820 | 19639 | 2344 | 21210 | 282 | 31 | 46325 |
| * nano-signal-slot tss | 3705 | 2563 | 7255 | 5901 | 11080 | 17082 | 2829 | 469 | 44615 |
| * dacap observable | 8421 | 1036 | 7667 | 6865 | 8612 | 12014 | 2839 | 99 | 38097 |
| copperspice cs_signal | 5370 | 1631 | 2315 | 2110 | 2316 | 9260 | 1002 | 0 | 17003 |
| neolib event | 11703 | 473 | 1813 | 3764 | 2204 | 4398 | 1013 | 0 | 13191 |
| * neolib event | 11889 | 480 | 1682 | 3352 | 2020 | 4285 | 942 | 84 | 12365 |
