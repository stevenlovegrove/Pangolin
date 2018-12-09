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
