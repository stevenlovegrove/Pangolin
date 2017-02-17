#include "pangolin/log/iPacketStream.hpp"
#include "pangolin/log/oPacketStream.hpp"
using namespace pangolin;


#include <sstream>
#include <iostream>
#include <unistd.h>
#include <thread>
#include <mutex>
using namespace std;

mutex output_m;
auto& output = cout;



void writeFakeFrame(const PacketStreamSource& source, size_t sequence_number, oPacketStream& target)
{
    stringstream r;
    r << "Hello, I am frame number " << sequence_number << " from source number " << source.id << ", named '" << source.driver << "'";

    json::value meta;
    meta["source"] = source.id;
    meta["frame number"] = sequence_number;
    meta["driver name"] = source.driver;

    target.writePacket(source.id, r.str().c_str(), r.str().size(), meta);
}


iPacketStream::FrameInfo readFakeFrame(PacketStreamSourceId id, iPacketStream& source, SyncTime* t)
{
    char buffer[1024];
//    source.lock();
    auto fi = source.nextFrame(id, t);
    if (fi.none())
    {
//        source.release();
        return fi;
    }
    source.readraw(buffer, fi.size);
//    source.release();

    output_m.lock();
    output << "Thread " << this_thread::get_id() <<  " read a frame from src " << fi.src << ", of size " << fi.size << "..." << endl;
    fi.meta.serialize(ostream_iterator<char>(output), true);
    output.write(buffer, fi.size);
    output << endl << endl;
    output_m.unlock();
    return fi;
}

void test_simple()
{
    oPacketStream write("test_simple");
    SyncTime t;

    PacketStreamSource video;
    video.driver = "video driver name";
    video.uri = "video driver fake uri";

    PacketStreamSource infrared;
    infrared.driver = "infrared driver name";
    infrared.uri = "infrared driver fake uri";

    write.addSource(video);
    write.addSource(infrared);

    output << "Opened an oPacketStream, writing some fake frames with a time delay to test sync: " << endl;
    for (size_t seqnum = 0; seqnum < 10; seqnum++)
    {
        sleep(1);
        writeFakeFrame(video, seqnum, write);
        output << ".";
        output.flush();
    }

    output << endl;
    write.writeEnd();
    write.close();
    sleep(1);

    output << "Done writing" << endl;

    iPacketStream read("test_simple");
    t.start();
    output << "Opened an iPacketStream, reading frames with a sync timer. You should see a 1 second delay between each frame." << endl;

    while(readFakeFrame(video.id, read, &t));

    output << "Done reading frames" << endl;
    output << "Testing seek... let's look for frame 5" << endl;

    if (!read.seek(video.id, 5, &t))
        throw runtime_error("Something terrible has happened.");

    if (!readFakeFrame(video.id, read, &t))
        throw runtime_error("Something terrible has happened.");

    output << "Now let's test resyncing, by reading through the other frames with timesync enabled. You should see the same 1 second delay." << endl;

    while(readFakeFrame(video.id, read, &t));

    output << "TEST COMPLETE" << endl << endl;

}

void test_parallel_multistream_singlefile()
{
    output << "Okay, now we test parallel processing with sync. " << endl
	    << "The easiest way is to open multiple iPacketStreams (each wraps its own filehandle), and read in parallel, using a shared SyncTime object." << endl
	    << endl
	    << "First, hang on while we make a new stream to test on: " << endl;

    oPacketStream write("test_parallel_multistream_singlefile");

    PacketStreamSource video;
    video.driver = "video driver name";
    video.uri = "video driver fake uri";

    PacketStreamSource infrared;
    infrared.driver = "infrared driver name";
    infrared.uri = "infrared driver fake uri";

    write.addSource(video);
    write.addSource(infrared);

    output << "Okay, now let's write some fake frames: three \"infrared\" frames between each \"video\" frame, each spaced by 1 second: " << endl;

    size_t jseq = 0;
    for (size_t i = 0; i < 10; ++i)
    {
	writeFakeFrame(video, i, write);
	output << ".";
	output.flush();
	sleep(1);
	for (size_t j = 0; j < 3; ++j)
	{
	    writeFakeFrame(infrared, jseq++, write);
	    output << ".";
	    output.flush();
	    sleep(1);
	}
    }

    write.writeEnd();
    write.close();
    sleep(1);

    output << endl << "All finished. Now we start our sync timer, and launch two reader threads: " << endl << endl;

    iPacketStream read1("test_parallel_multistream_singlefile");
    iPacketStream read2("test_parallel_multistream_singlefile");
    SyncTime t;

    thread video_reader([&video, &read1, &t]() { while(readFakeFrame(video.id, read1, &t)); });
    thread infrared_reader([&infrared, &read2, &t]() { while(readFakeFrame(infrared.id, read2, &t)); });

    video_reader.join();
    infrared_reader.join();

    output << "TEST COMPLETE" << endl << endl;
}

void test_parallel_multistream_multifile()
{
    output << "Next, we shall test sync two streams recorded at different times. This should work just fine, so long as the sync clock is started at the same time, or is shared between threads." << endl
	    << "Preparing stream #1: " << endl;

    oPacketStream write_vid("test_parallel_multistream_multifile_vid");
    PacketStreamSource video;
    video.driver = "video driver name";
    video.uri = "video driver fake uri";

    write_vid.addSource(video);
    for (size_t i = 0; i < 10; ++i)
    {
	writeFakeFrame(video, i, write_vid);
	output << ".";
	output.flush();
	sleep(4);
    }

    write_vid.writeEnd();
    write_vid.close();
    sleep(1);

    output << endl << "Preparing stream #2: " << endl;

    oPacketStream write_inf("test_parallel_multistream_multifile_inf");
    PacketStreamSource infrared;
    infrared.driver = "infrared driver name";
    infrared.uri = "infrared driver fake uri";

    write_inf.addSource(infrared);
    size_t jseq = 0;
    for (size_t i = 0; i < 10; ++i)
    {
	sleep(1);
	for (size_t j = 0; j < 3; ++j)
	{
	    writeFakeFrame(infrared, jseq++, write_inf);
	    output << ".";
	    output.flush();
	    sleep(1);
	}
    }
    write_inf.writeEnd();
    write_inf.close();

    output << endl << "All finished. Now we start our sync timer, and launch two reader threads: " << endl << endl;

    iPacketStream read_vid("test_parallel_multistream_multifile_vid");
    iPacketStream read_inf("test_parallel_multistream_multifile_inf");
    SyncTime t;

    thread video_reader([&video, &read_vid, &t]() { while(readFakeFrame(video.id, read_vid, &t)); });
    thread infrared_reader([&infrared, &read_inf, &t]() { while(readFakeFrame(infrared.id, read_inf, &t)); });
    video_reader.join();
    infrared_reader.join();
}

//Doesn't work yet
void test_parallel_singlestream_singlefile()
{
    output << "The complex way to do parallel processing involves multiple threads reading a single file. This may be necessary when we cannot open the file multiple times (socket or pipe), or hypothetically if we are very resource constrained."
	    << endl << endl
	    << "First, hang on while we make a new stream to test on: " << endl;

    oPacketStream write("test_parallel_singlestream_singlefile");

    PacketStreamSource video;
    video.driver = "video driver name";
    video.uri = "video driver fake uri";

    PacketStreamSource infrared;
    infrared.driver = "infrared driver name";
    infrared.uri = "infrared driver fake uri";

    write.addSource(video);
    write.addSource(infrared);

    output << "Okay, now let's write some fake frames: three \"infrared\" frames between each \"video\" frame, each spaced by 1 second: " << endl;

    size_t jseq = 0;
    for (size_t i = 0; i < 10; ++i)
    {
        writeFakeFrame(video, i, write);
        output << ".";
        output.flush();
        sleep(1);
        for (size_t j = 0; j < 3; ++j)
        {
            writeFakeFrame(infrared, jseq++, write);
            output << ".";
            output.flush();
            sleep(1);
        }
    }

    write.writeEnd();
    write.close();

    output << endl << "All finished. Now we start our sync timer, and launch two reader threads: " << endl << endl;

    iPacketStream read("test_parallel_singlestream_singlefile");
    SyncTime t;

    std::thread video_reader([&video, &read, &t]() { while(readFakeFrame(video.id, read, &t)); });
    std::thread infrared_reader([&infrared, &read, &t]() { while(readFakeFrame(infrared.id, read, &t)); });
    video_reader.join();
    infrared_reader.join();

}

int main (int, char**)
{
   output << "Some function tests for oPacketStream and iPacketStream. " << endl
	   << "These are just developer tests at this point, with human readable end-to-end function checks, rather than a formal set of unit test which returns pass/fail and provides full coverage. " << endl << endl;

   test_simple();
   test_parallel_multistream_singlefile();
   test_parallel_multistream_multifile();
//   test_parallel_singlestream_singlefile();

}



