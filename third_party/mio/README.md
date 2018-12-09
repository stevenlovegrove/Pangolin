# mio
An easy to use header-only cross-platform C++11 memory mapping library with an MIT license.

mio has been created with the goal to be easily includable (i.e. no dependencies) in any C++ project that needs memory mapped file IO without the need to pull in Boost.

Please feel free to open an issue, I'll try to address any concerns as best I can.

### Why?
Because memory mapping is the best thing since sliced bread!

More seriously, the primary motivation for writing this library instead of using Boost.Iostreams, was the lack of support for establishing a memory mapping with an already open file handle/descriptor. This is possible with mio.

Furthermore, Boost.Iostreams' solution requires that the user pick offsets exactly at page boundaries, which is cumbersome and error prone. mio, on the other hand, manages this internally, accepting any offset and finding the nearest page boundary.

Albeit a minor nitpick, Boost.Iostreams implements memory mapped file IO with a `std::shared_ptr` to provide shared semantics, even if not needed, and the overhead of the heap allocation may be unnecessary and/or unwanted.
In mio, there are two classes to cover the two use-cases: one that is move-only (basically a zero-cost abstraction over the system specific mmapping functions), and the other that acts just like its Boost.Iostreams counterpart, with shared semantics.

### How to create a mapping
NOTE: the file must exist before creating a mapping.

There are three ways to map a file into memory:

- Using the constructor, which throws a `std::system_error` on failure:
```c++
mio::mmap_source mmap(path, offset, size_to_map);
```
or you can omit the `offset` and `size_to_map` arguments, in which case the
entire file is mapped:
```c++
mio::mmap_source mmap(path);
```

- Using the factory function:
```c++
std::error_code error;
mio::mmap_source mmap = mio::make_mmap_source(path, offset, size_to_map, error);
```
or:
```c++
mio::mmap_source mmap = mio::make_mmap_source(path, error);
```

- Using the `map` member function:
```c++
std::error_code error;
mio::mmap_source mmap;
mmap.map(path, offset, size_to_map, error);
```
or:
```c++
mmap.map(path, error);
```
**NOTE:** The constructors **require** exceptions to be enabled. If you prefer
to build your projects with `-fno-exceptions`, you can still use the other ways.

Moreover, in each case, you can provide either some string type for the file's path, or you can use an existing, valid file handle.
```c++
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <mio/mmap.hpp>
#include <algorithm>

int main()
{
    // NOTE: error handling omitted for brevity.
    const int fd = open("file.txt", O_RDONLY);
    mio::mmap_source mmap(fd, 0, mio::map_entire_file);
    // ...
}
```
However, mio does not check whether the provided file descriptor has the same access permissions as the desired mapping, so the mapping may fail. Such errors are reported via the `std::error_code` out parameter that is passed to the mapping function.

**WINDOWS USERS**: This library *does* support the use of wide character types
for functions where character strings are expected (e.g. path parameters).

### Example

```c++
#include <mio/mmap.hpp>
#include <system_error> // for std::error_code
#include <cstdio> // for std::printf
#include <cassert>
#include <algorithm>
#include <fstream>

int handle_error(const std::error_code& error);
void allocate_file(const std::string& path, const int size);

int main()
{
    const auto path = "file.txt";

    // NOTE: mio does *not* create the file for you if it doesn't exist! You
    // must ensure that the file exists before establishing a mapping. It
    // must also be non-empty. So for illustrative purposes the file is
    // created now.
    allocate_file(path, 155);

    // Read-write memory map the whole file by using `map_entire_file` where the
    // length of the mapping is otherwise expected, with the factory method.
    std::error_code error;
    mio::mmap_sink rw_mmap = mio::make_mmap_sink(
            path, 0, mio::map_entire_file, error);
    if (error) { return handle_error(error); }

    // You can use any iterator based function.
    std::fill(rw_mmap.begin(), rw_mmap.end(), 'a');

    // Or manually iterate through the mapped region just as if it were any other 
    // container, and change each byte's value (since this is a read-write mapping).
    for (auto& b : rw_mmap) {
        b += 10;
    }

    // Or just change one value with the subscript operator.
    const int answer_index = rw_mmap.size() / 2;
    rw_mmap[answer_index] = 42;

    // Don't forget to flush changes to disk before unmapping. However, if
    // `rw_mmap` were to go out of scope at this point, the destructor would also
    // automatically invoke `sync` before `unmap`.
    rw_mmap.sync(error);
    if (error) { return handle_error(error); }

    // We can then remove the mapping, after which rw_mmap will be in a default
    // constructed state, i.e. this and the above call to `sync` have the same
    // effect as if the destructor had been invoked.
    rw_mmap.unmap();

    // Now create the same mapping, but in read-only mode. Note that calling the
    // overload without the offset and file length parameters maps the entire
    // file.
    mio::mmap_source ro_mmap;
    ro_mmap.map(path, error);
    if (error) { return handle_error(error); }

    const int the_answer_to_everything = ro_mmap[answer_index];
    assert(the_answer_to_everything == 42);
}

int handle_error(const std::error_code& error)
{
    const auto& errmsg = error.message();
    std::printf("error mapping file: %s, exiting...\n", errmsg.c_str());
    return error.value();
}

void allocate_file(const std::string& path, const int size)
{
    std::ofstream file(path);
    std::string s(size, '0');
    file << s;
}
```

`mio::basic_mmap` is move-only, but if multiple copies to the same mapping are needed, use `mio::basic_shared_mmap` which has `std::shared_ptr` semantics and has the same interface as `mio::basic_mmap`.
```c++
#include <mio/shared_mmap.hpp>

mio::shared_mmap_source shared_mmap1("path", offset, size_to_map);
mio::shared_mmap_source shared_mmap2(std::move(mmap1)); // or use operator=
mio::shared_mmap_source shared_mmap3(std::make_shared<mio::mmap_source>(mmap1)); // or use operator=
mio::shared_mmap_source shared_mmap4;
shared_mmap4.map("path", offset, size_to_map, error);
```

It's possible to define the type of a byte (which has to be the same width as `char`), though aliases for the most common ones are provided by default:
```c++
using mmap_source = basic_mmap_source<char>;
using ummap_source = basic_mmap_source<unsigned char>;

using mmap_sink = basic_mmap_sink<char>;
using ummap_sink = basic_mmap_sink<unsigned char>;
```
But it may be useful to define your own types, say when using the new `std::byte` type in C++17:
```c++
using mmap_source = mio::basic_mmap_source<std::byte>;
using mmap_sink = mio::basic_mmap_sink<std::byte>;
```

Though generally not needed, since mio maps users requested offsets to page boundaries, you can query the underlying system's page allocation granularity by invoking `mio::page_size()`, which is located in `mio/page.hpp`.

## CMake
As a header-only library, mio has no compiled components. Nevertheless, a [CMake](https://cmake.org/overview/) build system is provided to allow easy testing, installation, and subproject composition on many platforms and operating systems.

### Testing
Mio is distributed with a small suite of tests and examples.
When mio is configured as the highest level CMake project, this suite of executables is built by default.
Mio's test executables are integrated with the CMake test driver program, [CTest](https://cmake.org/cmake/help/latest/manual/ctest.1.html).

CMake supports a number of backends for compilation and linking.

To use a static configuration build tool, such as GNU Make or Ninja:

```sh
cd <mio source directory>
mkdir build
cd build

# Configure the build
cmake -D CMAKE_BUILD_TYPE=<Debug | Release> \
      -G <"Unix Makefiles" | "Ninja"> ..

# build the tests
< make | ninja | cmake --build . >

# run the tests
< make test | ninja test | cmake --build . --target test | ctest >
```

To use a dynamic configuration build tool, such as Visual Studio or Xcode:

```sh
cd <mio source directory>
mkdir build
cd build

# Configure the build
cmake -G <"Visual Studio 14 2015 Win64" | "Xcode"> ..

# build the tests
cmake --build . --config <Debug | Release>

# run the tests via ctest...
ctest --build-config <Debug | Release>

# ... or via CMake build tool mode...
cmake --build . --config <Debug | Release> --target test
```

Of course the **build** and **test** steps can also be executed via the **all** and **test** targets, respectively, from within the IDE after opening the project file generated during the configuration step.

Mio's testing is also configured to operate as a client to the [CDash](https://www.cdash.org/) software quality dashboard application. Please see the [Kitware documentation](https://cmake.org/cmake/help/latest/manual/ctest.1.html#dashboard-client) for more information on this mode of operation.

### Installation

Mio's build system provides an installation target and support for downstream consumption via CMake's [`find_package`](https://cmake.org/cmake/help/v3.0/command/find_package.html) intrinsic function.
CMake allows installation to an arbitrary location, which may be specified by defining `CMAKE_INSTALL_PREFIX` at configure time.
In the absense of a user specification, CMake will install mio to conventional location based on the platform operating system.

To use a static configuration build tool, such as GNU Make or Ninja:

```sh
cd <mio source directory>
mkdir build
cd build

# Configure the build
cmake [-D CMAKE_INSTALL_PREFIX="path/to/installation"] \
      [-D BUILD_TESTING=False]                         \
      -D CMAKE_BUILD_TYPE=Release                      \
      -G <"Unix Makefiles" | "Ninja"> ..

# install mio
<make install | ninja install | cmake --build . --target install>
```

To use a dynamic configuration build tool, such as Visual Studio or Xcode:

```sh
cd <mio source directory>
mkdir build
cd build

# Configure the project
cmake [-D CMAKE_INSTALL_PREFIX="path/to/installation"] \
      [-D BUILD_TESTING=False]                         \
      -G <"Visual Studio 14 2015 Win64" | "Xcode"> ..

# install mio
cmake --build . --config Release --target install
```

Note that the last command of the installation sequence may require administrator privileges (e.g. `sudo`) if the installation root directory lies outside your home directory.

This installation
+ copies the mio header files to the `include/mio` subdirectory of the installation root
+ generates and copies several CMake configuration files to the `share/cmake/mio` subdirectory of the installation root

This latter step allows downstream CMake projects to consume mio via `find_package`, e.g.

```cmake
find_package( mio REQUIRED )
target_link_libraries( MyTarget PUBLIC mio::mio )
```

**WINDOWS USERS**: The `mio::mio` target `#define`s `WIN32_LEAN_AND_MEAN` and `NOMINMAX`. The former ensures the imported surface area of the Win API is minimal, and the latter disables Windows' `min` and `max` macros so they don't intefere with `std::min` and `std::max`. Because *mio* is a header only library, these defintions will leak into downstream CMake builds. If their presence is causing problems with your build then you can use the alternative `mio::mio_full_winapi` target, which adds none of these defintions.

If mio was installed to a non-conventional location, it may be necessary for downstream projects to specify the mio installation root directory via either

+ the `CMAKE_PREFIX_PATH` configuration option,
+ the `CMAKE_PREFIX_PATH` environment variable, or
+ `mio_DIR` environment variable.

Please see the [Kitware documentation](https://cmake.org/cmake/help/v3.0/command/find_package.html) for more information.

In addition, mio supports packaged relocatable installations via [CPack](https://cmake.org/cmake/help/latest/manual/cpack.1.html).
Following configuration, from the build directory, invoke cpack as follows to generate a packaged installation:

```sh
cpack -G <generator name> -C Release
```

The list of supported generators varies from platform to platform. See the output of `cpack --help` for a complete list of supported generators on your platform.

### Subproject Composition
To use mio as a subproject, copy the mio repository to your project's dependencies/externals folder.
If your project is version controlled using git, a git submodule or git subtree can be used to syncronize with the updstream repository.
The [use](https://services.github.com/on-demand/downloads/submodule-vs-subtree-cheat-sheet/) and [relative advantages](https://andrey.nering.com.br/2016/git-submodules-vs-subtrees/) of these git facilities is beyond the scope of this document, but in brief, each may be established as follows:

```sh
# via git submodule
cd <my project's dependencies directory>
git submodule add -b master https://github.com/mandreyel/mio.git

# via git subtree
cd <my project's root directory>
git subtree add --prefix <path/to/dependencies>/mio       \
    https://github.com/mandreyel/mio.git master --squash
```

Given a mio subdirectory in a project, simply add the following lines to your project's to add mio include directories to your target's include path.

```cmake
add_subdirectory( path/to/mio/ )
target_link_libraries( MyTarget PUBLIC <mio::mio | mio> )
```

Note that, as a subproject, mio's tests and examples will not be built and CPack integration is deferred to the host project.
