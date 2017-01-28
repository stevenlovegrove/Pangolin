#include "pangolin/log/iPacketStream.hpp"
#include "pangolin/log/oPacketStream.hpp"
using namespace pangolin;


#include <sstream>
#include <iostream>
#include <unistd.h>
#include <thread>
#include <mutex>
using namespace std;

#include <sys/types.h>
#include <sys/stat.h>

void writeFakeFrame(const PacketStreamSource& source, size_t sequence_number, oPacketStream& target)
{
    stringstream r;
    r << "Hello, I am frame number " << sequence_number << " from source number " << source.id << ", named '" << source.driver << "'";

    json::value v;
    v["source id"] = source.id;
    v["source name"] = source.driver;
    v["frame number"] = sequence_number;
    target.writePacket(source.id, r.str().c_str(), r.str().size(), v);
}

void readFakeFrame(const iPacketStream::FrameInfo& fi, iPacketStream& source, ostream& target)
{
    char buffer[1024];
    target << "Reading a frame of size " << fi.size << "..." << endl;
    fi.meta.serialize(ostream_iterator<char>(target), true);
    source.readraw(buffer, fi.size);
    target.write(buffer, fi.size);
    target << endl << endl;
}

void test_pipe()
{
    if (mkfifo("./pipe", 0666))
    {
        static_cast<void>(system("rm ./pipe")+1);
        if (mkfifo("./pipe", 0666))
            throw runtime_error("Failed to create pipe");
    }

    cerr << "Testing basic pipe functionality..." << endl;
    sleep(2);


    thread writethread([]
    {
        cerr << "Write thread starting..." << endl;
        PacketStreamSource video;
        video.driver = "video driver name";
        video.uri = "video driver fake uri";
        oPacketStream write("pipe");
        write.addSource(video);

        for (size_t seqnum = 0; seqnum < 120; seqnum++)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            writeFakeFrame(video, seqnum, write);
        }

    });

    thread readthread([]
    {
        cout << "Read thread starting..." << endl;
        iPacketStream read("pipe");
        while (1)
        {
            auto fi = read.nextFrame(0, nullptr);
            if (fi.none())
            {
                read.release();
                break;
            }
            readFakeFrame(fi, read, cerr);
        }
    });

    writethread.join();
    readthread.join();

    static_cast<void>(system("rm ./pipe")+1);
}

void test_simple(std::ostream& target)
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

    target << "Opened an oPacketStream, writing some fake frames with a time delay to test sync: " << endl;
    for (size_t seqnum = 0; seqnum < 10; seqnum++)
    {
        sleep(1);
        writeFakeFrame(video, seqnum, write);
        target << ".";
        target.flush();
    }

    target << endl;
    write.writeEnd();
    write.close();
    sleep(1);

    target << "Done writing" << endl;

    iPacketStream read("test_simple");
    t.start();
    target << "Opened an iPacketStream, reading frames with a sync timer. You should see a 1 second delay between each frame." << endl;

    while (1)
    {
        auto fi = read.nextFrame(&t);
        if (fi.none())
            break;
        readFakeFrame(fi, read, target);
    }

    target << "Done reading frames" << endl;
    target << "Testing seek... let's look for frame 5" << endl;

    auto fi = read.seek(video.id, 5, &t);
    if (fi.none())
        throw runtime_error("Something terrible has happened.");

    fi = read.nextFrame(video.id, &t);
    readFakeFrame(fi, read, target);

    target << "Now let's test resyncing, by reading through the other frames with timesync enabled. You should see the same 1 second delay." << endl;
    while (1)
    {
        auto fi = read.nextFrame(&t);
        if (fi.none())
            break;
        readFakeFrame(fi, read, target);
    }
}

void test_parallel_multistream_singlefile(std::ostream& target)
{
    target << "Okay, now we test parallel processing with sync. " << endl
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

    target << "Okay, now let's write some fake frames: three \"infrared\" frames between each \"video\" frame, each spaced by 1 second: " << endl;

    size_t jseq = 0;
    for (size_t i = 0; i < 10; ++i)
    {
        writeFakeFrame(video, i, write);
        target << ".";
        target.flush();
        sleep(1);
        for (size_t j = 0; j < 3; ++j)
        {
            writeFakeFrame(infrared, jseq++, write);
            target << ".";
            target.flush();
            sleep(1);
        }
    }

    write.writeEnd();
    write.close();
    sleep(1);

    target << endl << "All finished. Now we start our sync timer, and launch two reader threads: " << endl << endl;

    iPacketStream read1("test_parallel_multistream_singlefile");
    iPacketStream read2("test_parallel_multistream_singlefile");
    SyncTime t;

    std::mutex target_lock;

    auto rff = [](PacketStreamSourceId src, iPacketStream& read, SyncTime& t, ostream& target, mutex& target_lock)
    {
        while(1)
        {
            read.lock();
            auto fi = read.nextFrame(src, &t);
            if (fi.none())
            {
                read.release();
                return;
            }
            target_lock.lock();
            target << "Thread " << this_thread::get_id() << ", reading source " << src << ", sez:" << endl;
            readFakeFrame(fi, read, target);
            read.release();
            target_lock.unlock();
        }
    };

    thread video_reader(rff, video.id, ref(read1), ref(t), ref(target), ref(target_lock));
    thread infrared_reader(rff, infrared.id, ref(read2), ref(t), ref(target), ref(target_lock));

    video_reader.join();
    infrared_reader.join();

    target << endl << "Now we can test seek. To remain synced when multiple iPacketStreams are open to the same file, we must seek() the same source/framenumber on each, using the same SyncTime object on each call." << endl
            << "Let's find frame 7 for source 0, then read as before, with each thread reading one source. " << endl;

    read1.seek(video.id, 7, &t);
    read2.seek(video.id, 7, &t);

    video_reader = thread(rff, video.id, ref(read1), ref(t), ref(target), ref(target_lock));
    infrared_reader = thread(rff, infrared.id, ref(read2), ref(t), ref(target), ref(target_lock));

    video_reader.join();
    infrared_reader.join();

}

void test_parallel_multistream_multifile(std::ostream& target)
{
    target << "Next, we shall test sync two streams recorded at different times. This should work just fine, so long as the sync clock is started at the same time, or is shared between threads." << endl
	    << "Preparing stream #1: " << endl;

    oPacketStream write_vid("test_parallel_multistream_multifile_vid");
    PacketStreamSource video;
    video.driver = "video driver name";
    video.uri = "video driver fake uri";
//    video.format_written = pangolin::ImageFileType::ImageFileTypeUnknown;

    write_vid.addSource(video);
    for (size_t i = 0; i < 10; ++i)
    {
	writeFakeFrame(video, i, write_vid);
	target << ".";
	target.flush();
	sleep(4);
    }

    write_vid.writeEnd();
    write_vid.close();
    sleep(1);

    target << endl << "Preparing stream #2: " << endl;

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
	    target << ".";
	    target.flush();
	    sleep(1);
	}
    }
    write_inf.writeEnd();
    write_inf.close();

    target << endl << "All finished. Now we start our sync timer, and launch two reader threads: " << endl << endl;

    iPacketStream read_vid("test_parallel_multistream_multifile_vid");
    iPacketStream read_inf("test_parallel_multistream_multifile_inf");
    SyncTime t;


    auto rff = [](PacketStreamSourceId src, iPacketStream& read, SyncTime& t, ostream& target, mutex& target_lock)
    {
        while(1)
        {
            read.lock();
            auto fi = read.nextFrame(src, &t);
            if (fi.none())
            {
                read.release();
                return;
            }
            target_lock.lock();
            target << "Thread " << this_thread::get_id() << ", reading source " << src << ", sez:" << endl;
            readFakeFrame(fi, read, target);
            read.release();
            target_lock.unlock();
        }
    };


    std::mutex target_lock;
    thread video_reader(rff, video.id, ref(read_vid), ref(t), ref(target), ref(target_lock));
    thread infrared_reader(rff, infrared.id, ref(read_inf), ref(t), ref(target), ref(target_lock));
    video_reader.join();
    infrared_reader.join();
}

//Doesn't work yet
//void test_parallel_singlestream_singlefile(std::ostream& target)
//{
//    target << "The complex way to do parallel processing involves multiple threads reading a single file. This may be necessary when we cannot open the file multiple times (socket or pipe), or hypothetically if we are very resource constrained."
//	    << endl << endl
//	    << "First, hang on while we make a new stream to test on: " << endl;
//
//    oPacketStream write("test_parallel_singlestream_singlefile");
//
//    PacketStreamSource video;
//    video.driver = "video driver name";
//    video.uri = "video driver fake uri";
//    video.format_written = pangolin::ImageFileType::ImageFileTypeUnknown;
//
//    PacketStreamSource infrared;
//    infrared.driver = "infrared driver name";
//    infrared.uri = "infrared driver fake uri";
//
//    write.addSource(video);
//    write.addSource(infrared);
//
//    target << "Okay, now let's write some fake frames: three \"infrared\" frames between each \"video\" frame, each spaced by 1 second: " << endl;
//
//    size_t jseq = 0;
//    for (size_t i = 0; i < 10; ++i)
//    {
//	writeFakeFrame(video, i, write);
//	target << ".";
//	target.flush();
//	sleep(1);
//	for (size_t j = 0; j < 3; ++j)
//	{
//	    writeFakeFrame(infrared, jseq++, write);
//	    target << ".";
//	    target.flush();
//	    sleep(1);
//	}
//    }
//
//    write.writeEnd();
//    write.close();
//
//    target << endl << "All finished. Now we start our sync timer, and launch two reader threads: " << endl << endl;
//
//    iPacketStream read("test_parallel_singlestream_singlefile");
//    SyncTime t;
//
//    std::mutex target_lock;
//    std::thread video_reader(readerThread, video.id, std::ref(read), std::ref(t), std::ref(target), std::ref(target_lock));
//    std::thread infrared_reader(readerThread, infrared.id, std::ref(read), std::ref(t), std::ref(target), std::ref(target_lock));
//    video_reader.join();
//    infrared_reader.join();
//
//}

int main (int, char**)
{
   cerr << "Some function tests for oPacketStream and iPacketStream. " << endl
	   << "These are just developer tests at this point, with human readable end-to-end function checks, rather than a formal set of unit test which returns pass/fail and provides full coverage. " << endl << endl;

   test_pipe();
   test_simple(cerr);
   test_parallel_multistream_singlefile(cerr);
   test_parallel_multistream_multifile(cerr);
//   test_parallel_singlestream_singlefile(cerr);

}



