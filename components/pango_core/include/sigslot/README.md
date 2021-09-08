# Sigslot, a signal-slot library

Sigslot is a header-only, thread safe implementation of signal-slots for C++.

## Features

The main goal was to replace Boost.Signals2.

Apart from the usual features, it offers

- Thread safety,
- Object lifetime tracking for automatic slot disconnection (extensible through ADL),
- RAII connection management,
- Slot groups to enforce slots execution order,
- Reasonable performance. and a simple and straightforward implementation.

Sigslot is unit-tested and should be reliable and stable enough to replace Boost Signals2.

The tests run cleanly under the address, thread and undefined behaviour sanitizers.

Many implementations allow signal return types, Sigslot does not because I have
no use for them. If I can be convinced of otherwise I may change my mind later on.

## Installation

No compilation or installation is required, just include `sigslot/signal.hpp`
and use it. Sigslot currently depends on a C++14 compliant compiler, but if need
arises it may be retrofitted to C++11. It is known to work with Clang 4.0 and GCC
5.0+ compilers on GNU Linux, MSVC 2017 and up, Clang-cl and MinGW on Windows.

However, be aware of a potential gotcha on Windows with MSVC and Clang-Cl compilers,
which may need the `/OPT:NOICF` linker flags in exceptional situations. Read The
Implementation Details chapter for an explanation.

A CMake list file is supplied for installation purpose and generating a CMake import
module. This is the preferred installation method. The `Pal::Sigslot` imported target
is available and already applies the needed linker flags. It is also required for
examples and tests, which optionally depend on Qt5 and Boost for adapters unit tests.

```cmake
# Using Sigslot from cmake
find_package(PalSigslot)

add_executable(MyExe main.cpp)
target_link_libraries(MyExe PRIVATE Pal::Sigslot)
```

A configuration option `SIGSLOT_REDUCE_COMPILE_TIME` is available at configuration
time. When activated, it attempts to reduce code bloat by avoiding heavy template
instantiations resulting from calls to `std::make_shared`.
This option is off by default, but can be activated for those who wish to favor
code size and compilation time at the expanse of slightly less efficient code.

Installation may be done using the following instructions from the root directory:

```sh
mkdir build && cd build
cmake .. -DSIGSLOT_REDUCE_COMPILE_TIME=ON -DCMAKE_INSTALL_PREFIX=~/local
cmake --build . --target install

# If you want to compile examples:
cmake --build . --target sigslot-examples

# And compile/execute unit tests:
cmake --build . --target sigslot-tests
```

### CMake FetchContent

`Pal::Sigslot` can also be integrated using the [FetchContent](https://cmake.org/cmake/help/latest/module/FetchContent.html) method.

```cmake
include(FetchContent)

FetchContent_Declare(
  sigslot
  GIT_REPOSITORY https://github.com/palacaze/sigslot
  GIT_TAG        19a6f0f5ea11fc121fe67f81fd5e491f2d7a4637 # v1.2.0
)
FetchContent_MakeAvailable(sigslot)

add_executable(MyExe main.cpp)
target_link_libraries(MyExe PRIVATE Pal::Sigslot)
```

## Documentation

Sigslot implements the signal-slot construct popular in UI frameworks, making it
easy to use the observer pattern or event-based programming. The main entry point
of the library is the `sigslot::signal<T...>` class template.

A signal is an object that can emit typed notifications, really values parametrized
after the signal class template parameters, and register any number of notification
handlers (callables) of compatible argument types to be executed with the values
supplied whenever a signal emission happens. In signal-slot parlance this is called
connecting a slot to a signal, where a "slot" represents a callable instance and
a "connection" can be thought of as a conceptual link from signal to slot.

All the snippets presented below are available in compilable source code form in
the example subdirectory.

### Basic usage

Here is a first example that showcases the most basic features of the library.

We first declare a parameter-free signal `sig`, then we proceed to connect several
slots and at last emit a signal which triggers the invocation of every slot callable
connected beforehand. Notice how The library handles diverse forms of callables.

```cpp
#include <sigslot/signal.hpp>
#include <iostream>

void f() { std::cout << "free function\n"; }

struct s {
    void m() { std::cout << "member function\n"; }
    static void sm() { std::cout << "static member function\n";  }
};

struct o {
    void operator()() { std::cout << "function object\n"; }
};

int main() {
    s d;
    auto lambda = []() { std::cout << "lambda\n"; };
    auto gen_lambda = [](auto && ...a) { std::cout << "generic lambda\n"; };

    // declare a signal instance with no arguments
    sigslot::signal<> sig;

    // connect slots
    sig.connect(f);
    sig.connect(&s::m, &d);
    sig.connect(&s::sm);
    sig.connect(o());
    sig.connect(lambda);
    sig.connect(gen_lambda);

    // emit a signal
    sig();
}
```

By default, the slot invocation order when emitting a signal is unspecified, please
do not rely on it being always the same. You may constrain a particular invocation
order by using slot groups, which are presented later on.

### Signal with arguments

That first example was simple but not so useful, let us move on to a signal that
emits values instead. A signal can emit any number of arguments, below.

```cpp
#include <sigslot/signal.hpp>
#include <iostream>
#include <string>

struct foo {
    // Notice how we accept a double as first argument here.
    // This is fine because float is convertible to double.
    // 's' is a reference and can thus be modified.
    void bar(double d, int i, bool b, std::string &s) {
        s = b ? std::to_string(i) : std::to_string(d);
    }
};

// Function objects can cope with default arguments and overloading.
// It does not work with static and member functions.
struct obj {
    void operator()(float, int, bool, std::string &, int = 0) {
        std::cout << "I was here\n";
    }

    void operator()() {}
};

int main() {
    // declare a signal with float, int, bool and string& arguments
    sigslot::signal<float, int, bool, std::string&> sig;

    // a generic lambda that prints its arguments to stdout
    auto printer = [] (auto a, auto && ...args) {
        std::cout << a;
        (void)std::initializer_list<int>{
            ((void)(std::cout << " " << args), 1)...
        };
        std::cout << "\n";
    };

    // connect the slots
    foo ff;
    sig.connect(printer);
    sig.connect(&foo::bar, &ff);
    sig.connect(obj());

    float f = 1.f;
    short i = 2;  // convertible to int
    std::string s = "0";

    // emit a signal
    sig(f, i, false, s);
    sig(f, i, true, s);
}
```

As shown, slots arguments types don't need to be strictly identical to the signal
template parameters, being convertible-from is fine. Generic arguments are fine too,
as shown with the `printer` generic lambda (which could have been written as a
function template too).

Right now there are two limitations that I can think of with respect to callable
handling: default arguments and function overloading. Both are working correctly
in the case of function objects but will fail to compile with static and member
functions, for different but related reasons.

#### Coping with overloaded functions

Consider the following piece of code:

```cpp
struct foo {
    void bar(double d);
    void bar();
};
```

What should `&foo::bar` refer to? As per overloading, this pointer over member
function does not map to a unique symbol, so the compiler won't be able to pick
the right symbol. One way of resolving the right symbol is to explicitly cast the
function pointer to the right function type. Here is an example that does just that
using a little helper tool for a lighter syntax (In fact I will probably add this
to the library soon).

```cpp
#include <sigslot/signal.hpp>

template <typename... Args, typename C>
constexpr auto overload(void (C::*ptr)(Args...)) {
    return ptr;
}

template <typename... Args>
constexpr auto overload(void (*ptr)(Args...)) {
    return ptr;
}

struct obj {
    void operator()(int) const {}
    void operator()() {}
};

struct foo {
    void bar(int) {}
    void bar() {}

    static void baz(int) {}
    static void baz() {}
};

void moo(int) {}
void moo() {}

int main() {
    sigslot::signal<int> sig;

    // connect the slots, casting to the right overload if necessary
    foo ff;
    sig.connect(overload<int>(&foo::bar), &ff);
    sig.connect(overload<int>(&foo::baz));
    sig.connect(overload<int>(&moo));
    sig.connect(obj());

    sig(0);

    return 0;
}
```

#### Coping with function with default arguments

Default arguments are not part of the function type signature, and can be redefined,
so they are really difficult to deal with. When connecting a slot to a signal, the
library determines if the supplied callable can be invoked with the signal argument
types, but at this point the existence of default function arguments is unknown
so there might be a mismatch in the number of arguments.

A simple work around for this use case would is to create a bind adapter, in fact
we can even make it quite generic like so:

```cpp
#include <sigslot/signal.hpp>

#define ADAPT(func) \
    [=](auto && ...a) { (func)(std::forward<decltype(a)>(a)...); }

void foo(int &i, int b = 1) {
    i += b;
}

int main() {
    int i = 0;

    // fine, all the arguments are handled
    sigslot::signal<int&, int> sig1;
    sig1.connect(foo);
    sig1(i, 2);

    // must wrap in an adapter
    i = 0;
    sigslot::signal<int&> sig2;
    sig2.connect(ADAPT(foo));
    sig2(i);

    return 0;
}
```

### Connection management

#### Connection object

What was not made apparent until now is that `signal::connect()` actually returns
a `sigslot::connection` object that may be used to manage the behaviour and lifetime
of a signal-slot connection. `sigslot::connection` is a lightweight object (basically
a `std::weak_ptr`) that allows interaction with an ongoing signal-slot connection
and exposes the following features:

- Status querying, that is testing whether a connection is valid, ongoing or facing destruction,
- Connection (un)blocking, which allows to temporarily disable the invocation of a slot when a signal is emitted,
- Disconnection of a slot, the destruction of a connection previously created via `signal::connect()`.

A `sigslot::connection` does not tie a connection to a scope: this is not a RAII
object, which explains why it can be copied. It can be however implicitly converted
into a `sigslot::scoped_connection` which destroys the connection when going out
of scope.

Here is an example illustrating some of those features:

```cpp
#include <sigslot/signal.hpp>
#include <string>

int i = 0;

void f() { i += 1; }

int main() {
    sigslot::signal<> sig;

    // keep a sigslot::connection object
    auto c1 = sig.connect(f);

    // disconnection
    sig();  // i == 1
    c1.disconnect();
    sig();  // i == 1

    // scope based disconnection
    {
        sigslot::scoped_connection sc = sig.connect(f);
        sig();  // i == 2
    }

    sig();  // i == 2;


    // connection blocking
    auto c2 = sig.connect(f);
    sig();  // i == 3
    c2.block();
    sig();  // i == 3
    c2.unblock();
    sig();  // i == 4
}
```

#### Extended connection signature

Sigslot supports an extended slot signature with an additional `sigslot::connection`
reference as first argument, which permits connection management from inside the
slot. This extended signature is accessible using the `connect_extended()` method.

```cpp
#include <sigslot/signal.hpp>

int main() {
    int i = 0;
    sigslot::signal<> sig;

    // extended connection
    auto f = [](auto &con) {
        i += 1;             // do work
        con.disconnect();   // then disconnects
    };

    sig.connect_extended(f);
    sig();  // i == 1
    sig();  // i == 1 because f was disconnected
}
```

#### Automatic slot lifetime tracking

The user must make sure that the lifetime of a slot exceeds the one of a signal,
which may get tedious in complex software. To simplify this task, Sigslot can
automatically disconnect slot object whose lifetime it is able to track. In order
to do that, the slot must be convertible to a weak pointer of some form.

`std::shared_ptr` and `std::weak_ptr` are supported out of the box, and adapters
are provided to support `boost::shared_ptr`, `boost::weak_ptr` and Qt `QSharedPointer`,
`QWeakPointer` and any class deriving from `QObject`.

Other trackable objects can be added by declaring a `to_weak()` adapter function.

```cpp
#include <sigslot/signal.hpp>
#include <sigslot/adapter/qt.hpp>

int sum = 0;

struct s {
    void f(int i) { sum += i; }
};

class MyObject : public QObject {
    Q_OBJECT
public:
    void add(int i) const { sum += i; }
};

int main() {
    sum = 0;
    signal<int> sig;

    // track lifetime of object and also connect to a member function
    auto p = std::make_shared<s>();
    sig.connect(&s::f, p);

    sig(1);     // sum == 1
    p.reset();
    sig(1);     // sum == 1

    // track an unrelated object lifetime
    struct dummy;
    auto l = [&](int i) { sum += i; };

    auto d = std::make_shared<dummy>();
    sig.connect(l, d);
    sig(1);     // sum == 2
    d.reset();
    sig(1);     // sum == 2

    // track a QObject
    {
        MyObject o;
        sig.connect(&MyObject::add, &o);

        sig(1); // sum == 3
    }

    sig(1);     // sum == 3
}
```

#### Intrusive slot lifetime tracking

Another way of ensuring automatic disconnection of pointer over member functions
slots is by explicitly inheriting from `sigslot::observer` or `sigslot::observer_st`.
The former is thread-safe, contrary to the later.

Here is an example usage.

```cpp
#include <sigslot/signal.hpp>

int sum = 0;

struct s : sigslot::observer_st {
    void f(int i) { sum += i; }
};

struct s_mt : sigslot::observer {
    ~s_mt() {
        // Needed to ensure proper disconnection prior to object destruction
        // in multithreaded contexts.
        this->disconnect_all();
    }
    
    void f(int i) { sum += i; }
};

int main() {
    sum = 0;
    signal<int> sig;
    
    {
        // Lifetime of object instance p is tracked
        s p;
        s_mt pm;
        sig.connect(&s::f, &p);
        sig.connect(&s_mt::f, &pm);
        sig(1);     // sum == 2
    }
    
    // The slots got disconnected at instance destruction
    sig(1);         // sum == 2
}
```

The objects that use this intrusive approach may be connected to any number of
unrelated signals.

### Disconnection without a connection object

Support for slot disconnection by supplying an appropriate function signature,
object pointer or tracker has been introduced in version 1.2.0.

One can disconnect any number of slots using the `signal::disconnect()` method,
which proposes 4 overloads to specify the disconnection criterion:

- The first takes a reference to a callable. Any kind of callable can be passed,
  even pointers to member functions, function objects and lambdas,
- The second takes a pointer to an object, for slots bound to a pointer to member
  function, or a tracking object,
- The third overload takes both kinds of arguments at the same time and can be
  used to pinpoint a specific pair of object + callable.
- The last overload takes a group id and disconnects all the slots in this group.

Disconnection of lambdas is only possible for lambdas bound to a variable, due
to their uniqueness.

The second overload currently needs RTTI to disconnect from pointers to member
functions, function objects and lambdas. This limitation does not apply to free
and static member functions. The reasons stems from the fact that in C++, pointers
to member functions of unrelated types are not comparable, contrary to pointers to
free and static member functions. For instance, the pointer to member functions of
virtual methods of different classes can have the same address (they kind of store
the offset of the method into the vtable).

However, Sigslot can be compiled with RTTI disabled and the overload will be
deactivated for problematic cases.

As a side node, this feature admittedly added more code than anticipated at first
because it is a tricky and easy to get wrong. It has been designed carefully, with
correctness in mind, and does not have any hidden costs unless you actually use it.

Here is an example demonstrating the feature.

```cpp
#include <sigslot/signal.hpp>
#include <string>

static int i = 0;

void f1() { i += 1; }
void f2() { i += 1; }

struct s {
    void m1() { i += 1; }
    void m2() { i += 1; }
    void m3() { i += 1; }
};

struct o {
    void operator()() { i += 1; }
};

int main() {
    sigslot::signal<> sig;
    s s1;
    auto s2 = std::make_shared<s>();

    auto lbd = [&] { i += 1; };

    sig.connect(f1);           // #1
    sig.connect(f2);           // #2
    sig.connect(&s::m1, &s1);  // #3
    sig.connect(&s::m2, &s1);  // #4
    sig.connect(&s::m3, &s1);  // #5
    sig.connect(&s::m1, s2);   // #6
    sig.connect(&s::m2, s2);   // #7
    sig.connect(o{});          // #8
    sig.connect(lbd);          // #9

    sig();  // i == 9

    sig.disconnect(f2);              // #2 is removed
    sig.disconnect(&s::m1);          // #3 and #6 are removed
    sig.disconnect(o{});             // #8 and is removed
 // sig.disconnect(&o::operator());  // same as the above, more efficient
    sig.disconnect(lbd);             // #9 and is removed
    sig.disconnect(s2);              // #7 is removed
    sig.disconnect(&s::m3, &s1);     // #5 is removed, not #4

    sig();  // i == 11

    sig.disconnect_all();         // remove all remaining slots
    return 0;
}
```

### Enforcing slot invocation order with slot groups

From version 1.2.0, slots can be assigned a group id in order to control the
relative order of invocation of slots.

The order of invocation of slots in a same group is unspecified and should not be
relied upon, however slot groups are invoked in ascending group id order.
When the group id of a slot is not set, it is assigned to the group 0.
Group ids can have any value in the range of signed 32 bit integers.

```cpp
#include <sigslot/signal.hpp>
#include <cstdio>
#include <limits>

int main() {
    sigslot::signal<> sig;

    // simply assigning a group id as last argument to connect
    sig.connect([] { std::puts("Second"); }, 1);
    sig.connect([] { std::puts("Last"); }, std::numeric_limits<sigslot::group_id>::max());
    sig.connect([] { std::puts("First"); }, -10);
    sig();

    return 0;
}
```

### Thread safety

Thread safety is unit-tested. In particular, cross-signal emission and recursive
emission run fine in a multiple threads scenario.

`sigslot::signal` is a typedef to the more general `sigslot::signal_base` template
class, whose first template argument must be a Lockable type. This type will dictate
the locking policy of the class.

Sigslot offers 2 typedefs,

- `sigslot::signal` usable from multiple threads and uses std::mutex as a lockable.
  In particular, connection, disconnection, emission and slot execution are thread
  safe. It is also safe with recursive signal emission.
- `sigslot::signal_st` is a non thread-safe alternative, it trades safety for slightly
  faster operation.


## Implementation details

### Using function pointers to disconnect slots

Comparing function pointers is a nightmare in C++. Here is a table demonstrating
the size and address of a variety of cases as a showcase:

```cpp
void fun() {}

struct b1 {
    virtual ~b1() = default;
    static void sm() {}
    void m() {}
    virtual void vm() {}
};

struct b2 {
    virtual ~b2() = default;
    static void sm() {}
    void m() {}
    virtual void vm() {}
};

struct c {
    virtual ~c() = default;
    virtual void w() {}
};

struct d : b1 {
    static void sm() {}
    void m() {}
    void vm() override {}
};

struct e : b1, c {
    static void sm() {}
    void m() {}
    void vm() override{}
};
```

| Symbol  | GCC 9 Linux 64<br>Sizeof | GCC 9 Linux 64<br>Address | MSVC 16.6 32<br>Sizeof | MSVC 16.6 32<br>Address | GCC 8 Mingw 32<br>Sizeof | GCC 8 Mingw 32<br>Address | Clang-cl 9 32<br>Sizeof | Clang-cl 9 32<br>Address |
|---------|--------------------------|---------------------------|------------------------|-------------------------|--------------------------|---------------------------|-------------------------|--------------------------|
| fun     | 8                        | 0x802340                  | 4                      | 0x1311A6                | 4                        | 0xF41540                  | 4                       | 0x0010AE                 |
| &b1::sm | 8                        | 0xE03140                  | 4                      | 0x7612A5                | 4                        | 0x308D40                  | 4                       | 0x0010AE                 |
| &b1::m  | 16                       | 0xF03240                  | 4                      | 0x1514A5                | 8                        | 0x248D40                  | 4                       | 0x0010AE                 |
| &b1::vm | 16                       | 0x11                      | 4                      | 0x9F11A5                | 8                        | 0x09                      | 4                       | 0x8023AE                 |
| &b2::sm | 8                        | 0x003340                  | 4                      | 0xA515A5                | 4                        | 0x408D40                  | 4                       | 0x0010AE                 |
| &b2::m  | 16                       | 0x103440                  | 4                      | 0xEB10A5                | 8                        | 0x348D40                  | 4                       | 0x0010AE                 |
| &b2::vm | 16                       | 0x11                      | 4                      | 0x6A14A5                | 8                        | 0x09                      | 4                       | 0x8023AE                 |
| &d::sm  | 8                        | 0x203440                  | 4                      | 0x2612A5                | 4                        | 0x108D40                  | 4                       | 0x0010AE                 |
| &d::m   | 16                       | 0x303540                  | 4                      | 0x9D13A5                | 8                        | 0x048D40                  | 4                       | 0x0010AE                 |
| &d::vm  | 16                       | 0x11                      | 4                      | 0x4412A5                | 8                        | 0x09                      | 4                       | 0x8023AE                 |
| &e::sm  | 8                        | 0x403540                  | 4                      | 0xF911A5                | 4                        | 0x208D40                  | 4                       | 0x0010AE                 |
| &e::m   | 16                       | 0x503640                  | 8                      | 0x8111A5                | 8                        | 0x148D40                  | 8                       | 0x0010AE                 |
| &e::vm  | 16                       | 0x11                      | 8                      | 0xA911A5                | 8                        | 0x09                      | 8                       | 0x8023AE                 |

MSVC and Clang-cl in Release mode optimize functions with the same definition by
merging them. This is a behaviour that can be deactivated with the `/OPT:NOICF`
linker option.
Sigslot tests and examples rely on a lot a identical callables which trigger this
behaviour, which is why it deactivates this particular optimization on the affected
compilers.

### Known bugs

Using generic lambdas with GCC less than version 7.4 can trigger [Bug #68071](https://gcc.gnu.org/bugzilla/show_bug.cgi?id=68071).
