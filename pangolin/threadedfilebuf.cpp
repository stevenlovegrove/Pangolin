/* This file is part of the Pangolin Project.
 * http://github.com/stevenlovegrove/Pangolin
 *
 * Copyright (c) 2011 Steven Lovegrove
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include "threadedfilebuf.h"

#include <cstring>

using namespace std;

namespace pangolin
{

threadedfilebuf::threadedfilebuf( const std::string& filename, unsigned int buffer_size_bytes )
    : mem_buffer(0), mem_size(0), mem_start(0), mem_end(0)
{
    file.open(filename.c_str(), ios::out | ios::binary );

    mem_max_size = buffer_size_bytes;
    mem_buffer = new char[mem_max_size];

    write_thread = boost::thread(boost::ref(*this));
}

threadedfilebuf::~threadedfilebuf()
{
    if( write_thread.joinable() )
    {
      write_thread.interrupt();
      write_thread.join();
    }

    if( mem_buffer) delete mem_buffer;
    file.close();
}

std::streamsize threadedfilebuf::xsputn(const char* data, std::streamsize num_bytes)
{
    if( num_bytes > mem_max_size )
        throw exception();

    {
        boost::unique_lock<boost::mutex> lock(update_mutex);

        // wait until there is space to write into buffer
        while( mem_size + num_bytes > mem_max_size ) {
            cond_dequeued.wait(lock);
        }

        // add image to end of mem_buffer
        const int array_a_size =
            (mem_start <= mem_end) ? (mem_max_size - mem_end) : (mem_start - mem_end);

        if( num_bytes <= array_a_size )
        {
            // copy in one
            memcpy(mem_buffer + mem_end, data, num_bytes);
            mem_end += num_bytes;
            mem_size += num_bytes;
        }else{
            const int array_b_size = num_bytes - array_a_size;
            memcpy(mem_buffer + mem_end, data, array_a_size);
            memcpy(mem_buffer, data+array_a_size, array_b_size);
            mem_end = array_b_size;
            mem_size += num_bytes;
        }

        if(mem_end == mem_max_size)
            mem_end = 0;
    }

    cond_queued.notify_one();

    return num_bytes;
}

void threadedfilebuf::operator()()
{
    int data_to_write = 0;

    while(true)
    {
        {
            boost::unique_lock<boost::mutex> lock(update_mutex);

            while( mem_size == 0 )
                cond_queued.wait(lock);

            data_to_write =
                    (mem_start < mem_end) ?
                        mem_end - mem_start :
                        mem_max_size - mem_start;
        }

        std::streamsize bytes_written =
            file.sputn(mem_buffer + mem_start, data_to_write );

        if( bytes_written != data_to_write)
            throw std::exception();

        {
            boost::unique_lock<boost::mutex> lock(update_mutex);

            mem_size -= data_to_write;
            mem_start += data_to_write;

            if(mem_start == mem_max_size)
                mem_start = 0;
        }

        cond_dequeued.notify_all();
    }
}

}
