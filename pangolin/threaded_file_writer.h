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

#ifndef PANGOLIN_THREADED_WRITE_H
#define PANGOLIN_THREADED_WRITE_H

#include <iostream>
#include <fstream>

#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>

namespace pangolin
{

class ThreadedFileWriter
{
public:
    ThreadedFileWriter(const std::string& filename, unsigned int buffer_size_bytes);
    ~ThreadedFileWriter();

    // Asynchronous write
    int write(char* data, int num_bytes);

    void operator()();

    std::ofstream file;
protected:
    char* mem_buffer;
    int mem_size;
    int mem_max_size;
    int mem_start;
    int mem_end;

    boost::thread write_thread;
    boost::mutex update_mutex;
    boost::condition cond_queued;
    boost::condition cond_dequeued;
};

}


#endif // PANGOLIN_THREADED_WRITE_H
