# stduuid
A C++ cross-platform single-header library implementation **for universally unique identifiers**, simply know as either UUID or GUID (mostly on Windows). A UUID is a 128-bit number used to uniquely identify information in computer systems, such as database table keys, COM interfaces, classes and type libraries, and many others.

[![Build Status](https://travis-ci.org/mariusbancila/stduuid.svg?branch=master)](https://travis-ci.org/mariusbancila/stduuid)
[![Tests status](https://ci.appveyor.com/api/projects/status/0kw1n2s2xqxu5m62?svg=true&pendingText=tests%20-%20pending&failingText=tests%20-%20FAILED&passingText=tests%20-%20OK)](https://ci.appveyor.com/project/mariusbancila/stduuid)

For information about UUID/GUIDs see:
* [Universally unique identifier](https://en.wikipedia.org/wiki/Universally_unique_identifier)
* [A Universally Unique IDentifier (UUID) URN Namespace](https://www.ietf.org/rfc/rfc4122.txt)

## Library overview
Although the specification puts the uuid library in the `std` namespace, this implementation uses the namespace `uuids` for this purpose, in order to make the library usable without violating the restrictions imposed on the `std` namespace. The following types and utilities are available:

Basic types:

| Name | Description |
| ---- | ----------- |
| `uuid` | a class representing a UUID; this can be default constructed (a nil UUID), constructed from a range (defined by a pair of iterators), or from a `span`. |
| `uuid_variant` | a strongly type enum representing the type of a UUID |
| `uuid_version` | a strongly type enum representing the version of a UUID |
| `uuid_error` | a class representing an exception type for `uuid` operations |

Generators:

| Name | Description |
| ---- | ----------- |
| `uuid_system_generator` | a function object that generates new UUIDs using operating system resources (`CoCreateGuid` on Windows, `uuid_generate` on Linux, `CFUUIDCreate` on Mac) |
| `basic_uuid_random_generator` | a function object that generates version 4 UUIDs using a pseudo-random number generator engine. |
| `uuid_random_generator` | a basic_uuid_random_generator using the Marsenne Twister engine, i.e. `basic_uuid_random_generator<std::mt19937>` |
| `uuid_name_generator` | a function object that generates version 5, name-based UUIDs using SHA1 hashing. |

Utilities:

| Name | Description |
| ---- | ----------- |
| `std::swap<>` | specialization of `swap` for `uuid` |
| `std::hash<>` | specialization of `hash` for `uuid` (necessary for storing UUIDs in unordered associative containers, such as `std::unordered_set`) |

Other:

| Name | Description |
| ---- | ----------- |
| `operator==` and `operator!=` | for UUIDs comparison for equality/inequality |
| `operator<` | for comparing whether one UUIDs is less than another. Although this operation does not make much logical sense, it is necessary in order to store UUIDs in a std::set. |
| `operator<<` | to write a UUID to an output stream using the canonical textual representation. |
| `to_string()` | creates a `std::string` with the canonical textual representation of a UUID. |
| `to_wstring()` | creates a `std::wstring` with the canonical textual representation of a UUID. |

This project is currently under development and should be ignored until further notice.

## Library history
This library is an implementation of the proposal [P0959](P0959.md). As the proposal evolves based on the standard commity and the C++ community feedback, this library implementation will reflect those changes. See the revision history of the proposal for history of changes.

## Using the library
The following is a list of examples for using the library:
* Creating a nil UUID

```cpp
uuid empty;
assert(empty.is_nil());
assert(empty.size() == 16);
```

* Creating a new UUID

```cpp
uuid const id = uuids::uuid_system_generator{}();
assert(!id.is_nil());
assert(id.size() == 16);
assert(id.version() == uuids::uuid_version::random_number_based);
assert(id.variant() == uuids::uuid_variant::rfc);
```

* Creating a new UUID with a default random generator

```cpp
uuids::uuid_random_generator gen;
uuid const id = gen();
assert(!id.is_nil());
assert(id.size() == 16);
assert(id.version() == uuids::uuid_version::random_number_based);
assert(id.variant() == uuids::uuid_variant::rfc);
```

* Creating a new UUID with a particular random generator

```cpp
std::random_device rd;
std::ranlux48_base generator(rd());
uuids::basic_uuid_random_generator<std::ranlux48_base> gen(&generator);
uuid const id = gen();
assert(!id.is_nil());
assert(id.size() == 16);
assert(id.version() == uuids::uuid_version::random_number_based);
assert(id.variant() == uuids::uuid_variant::rfc);
```

* Creating a new UUID with the name generator

```cpp
uuids::uuid_name_generator gen;
uuid const id = gen();

assert(!id.is_nil());
assert(id.size() == 16);
assert(id.version() == uuids::uuid_version::name_based_sha1);
assert(id.variant() == uuids::uuid_variant::rfc);
```

* Create a UUID from a string

```cpp
using namespace std::string_literals;

auto str = "47183823-2574-4bfd-b411-99ed177d3e43"s;
uuid id(uuids::uuid::from_string(str));
assert(uuids::to_string(id) == str);

// or

uuid id(uuids::uuid::from_string(L"{47183823-2574-4bfd-b411-99ed177d3e43}"s));
assert(id.wstring() == str);
```

* Creating a UUID from sequence of 16 bytes

```cpp
std::array<uuids::uuid::value_type, 16> arr{{
   0x47, 0x18, 0x38, 0x23,
   0x25, 0x74,
   0x4b, 0xfd,
   0xb4, 0x11,
   0x99, 0xed, 0x17, 0x7d, 0x3e, 0x43}};
uuid id(arr);

assert(uuids::to_string(id) == "47183823-2574-4bfd-b411-99ed177d3e43");

// or

uuids::uuid::value_type arr[16] = {
   0x47, 0x18, 0x38, 0x23,
   0x25, 0x74,
   0x4b, 0xfd,
   0xb4, 0x11,
   0x99, 0xed, 0x17, 0x7d, 0x3e, 0x43 };
uuid id(std::begin(arr), std::end(arr));
assert(uuids::to_string(id) == "47183823-2574-4bfd-b411-99ed177d3e43");
```
* Comparing UUIDs

```cpp
uuid empty;
uuid id = uuids::uuid_system_generator{}();
assert(empty == empty);
assert(id == id);
assert(empty != id);
```

* Swapping UUIDs

```cpp
uuid empty;
uuid id = uuids::uuid_system_generator{}();

assert(empty.is_nil());
assert(!id.is_nil());

std::swap(empty, id);

assert(!empty.is_nil());
assert(id.is_nil());

empty.swap(id);

assert(empty.is_nil());
assert(!id.is_nil());
```
* Converting to string

```cpp
uuid empty;
assert(uuids::to_string(empty) == "00000000-0000-0000-0000-000000000000");
assert(uuids::to_wstring(empty) == L"00000000-0000-0000-0000-000000000000");
```

* Iterating through the UUID data

```cpp
std::array<uuids::uuid::value_type, 16> arr{{
   0x47, 0x18, 0x38, 0x23,
   0x25, 0x74,
   0x4b, 0xfd,
   0xb4, 0x11,
   0x99, 0xed, 0x17, 0x7d, 0x3e, 0x43}};

uuid id(arr);

size_t i = 0;
for (auto const & b : id)
   assert(arr[i++] == b);
```

* Using with an orderered associative container

```cpp
uuids::uuid_random_generator gen;
std::set<uuids::uuid> ids{uuid{}, gen(), gen(), gen(), gen()};

assert(ids.size() == 5);
assert(ids.find(uuid{}) != ids.end());
```

* Using in an unordered associative container

```cpp
uuids::uuid_random_generator gen;
std::unordered_set<uuids::uuid> ids{uuid{}, gen(), gen(), gen(), gen()};

assert(ids.size() == 5);
assert(ids.find(uuid{}) != ids.end());
```

* Hashing UUIDs

```cpp
using namespace std::string_literals;
auto str = "47183823-2574-4bfd-b411-99ed177d3e43"s;
uuid id(uuids::uuid::from_string(str));

auto h1 = std::hash<std::string>{};
auto h2 = std::hash<uuid>{};
assert(h1(str) == h2(id));
```

## Support
The library is supported on all major operating systems: Windows, Linux and Mac OS.

## Dependencies
Because no major compiler supports `std::span` yet the [Microsoft Guidelines Support Library](https://github.com/Microsoft/GSL) (aka GSL) is used for its span implementation (from which the standard version was defined). 

## Testing
A testing project is available in the sources. To build and execute the tests do the following:
* Clone or download this repository
* Create a `build` directory in the root directory of the sources
* Run the command `cmake ..` from the `build` directory; if you do not have CMake you must install it first.
* Build the project created in the previous step
* Run the executable.

## Credits
The SHA1 implementation is based on the [TinySHA1](https://github.com/mohaps/TinySHA1) library.
