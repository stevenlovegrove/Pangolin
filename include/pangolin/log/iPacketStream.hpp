
#pragma once

#include <mutex>
#include <fstream>
#include <thread>
//#include <condition_variable>

#include <pangolin/utils/timer.h>
#include <pangolin/utils/file_utils.h>
#include <pangolin/log/PacketStream.hpp>

namespace pangolin
{

//lightweight timestamp class to allow synchronized playback from the same (or a different) stream.
//All playback functions called with the same SyncTime will be time-synchronized, and will remain synchronized on seek() if the SyncTime is passed in when seeking.
//Playback with multiple SyncTimes (on the same or different streams) should also be synced, even in different processes or systems (underlying clock sync is not required).
//However, playback with multiple SyncTimes will break on seek().
class PANGOLIN_EXPORT SyncTime
{
	int64_t _start;
	std::mutex _startlock;
    public:
	SyncTime() :_start(TimeNow_us()){};
	void start(){std::lock_guard<decltype(_startlock)> lg(_startlock);_start = TimeNow_us();};
	void waitUntilOffset(decltype(_start) stream_time_offset) const
	{
	    const auto viewer_time_offset = TimeNow_us() - _start;
	    if (viewer_time_offset < stream_time_offset)
	    	std::this_thread::sleep_for(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::microseconds(stream_time_offset - viewer_time_offset)));
	};
	void resyncToOffset(decltype(_start) stream_time_offset) {std::lock_guard<decltype(_startlock)> lg(_startlock);_start = TimeNow_us() - stream_time_offset;};
};

class PANGOLIN_EXPORT iPacketStream
{
    public:
	//nextFrame() functions return this. Contains information about the queued-up frame. Relies on RVO.
	struct FrameInfo
	{
	    PacketStreamSourceId src;
	    int64_t time;
	    size_t size;
	    size_t sequence_num;
	    std::streampos stream_location;
	    json::value meta;
	    FrameInfo(): src(static_cast<decltype(src)>(-1)), time(-1), size(static_cast<decltype(size)>(-1)), sequence_num(static_cast<decltype(sequence_num)>(-1)){};
	    bool none() const {return src == static_cast<decltype(src)>(-1);};
	    operator bool() const {return !none();};
	};

    private:
	class Stream: public std::ifstream
	{
	private:
	    using parent = std::ifstream;
	    bool _seekable;
	    pangoTagType _tag;
	    FrameInfo _frame;
	    size_t _data_len; //Amount of frame data left to read. Tracks our position within a data block.

	    void cclear() {_data_len = 0; _tag = 0;_frame.src = static_cast<decltype(_frame.src)>(-1);}

	public:
	    Stream(): _seekable(false){cclear();};
	    Stream(const std::string& filename):parent(filename.c_str(), std::ios::in | std::ios::binary), _seekable(!IsPipe(filename)){cclear();}

	    bool seekable() const {return _seekable;};
	    size_t data_len() const {return _data_len;};
	    void data_len(size_t d) {_data_len = d;};

	    void open(const std::string& filename){close(); parent::open(filename.c_str(), std::ios::in | std::ios::binary); _seekable = !IsPipe(filename);};
	    void close(){cclear(); if (parent::is_open()) parent::close();};

        void seekg(std::streampos target);
        void seekg(std::streamoff off, std::ios_base::seekdir way);
        std::streampos tellg();
        size_t read(char* target, size_t len);
        char get();
        size_t skip(size_t len);

        size_t readUINT();
        int64_t readTimestamp();
        pangoTagType peekTag();
        pangoTagType readTag();
        pangoTagType readTag(pangoTagType);
        pangoTagType syncToTag();

        FrameInfo peekFrameHeader(const iPacketStream&);
        FrameInfo readFrameHeader(const iPacketStream&);
	};

	sourceIndexType _sources;
	std::vector<size_t> _next_packet_framenum;
	packetIndex _index;

	Stream _stream;
	std::recursive_mutex _lock;
	int64_t _starttime;

//	std::vector<decltype(std::this_thread::get_id())> _subscribers;
//	std::vector<std::unique_ptr<std::condition_variable_any>> _frame_ready;


    public:
	iPacketStream():_starttime(0){};
	iPacketStream(const std::string& filename): _stream(filename), _starttime(0) {init();};
	void open(const std::string& filename) {_starttime = 0; _stream.open(filename); init();}
	void close(){_stream.close(); _sources.clear();}

	~iPacketStream(){close();};

	const decltype(_sources)& sources() const {return _sources;};

	//user is responsible for locking, since we cannot know the desired locking behaviour
	void lock() {_lock.lock();};
	void release() {_lock.unlock();};
	decltype(_lock)& mutex() {return _lock;}; //exposes the underlying mutex... this allows std::lock_guard, and similar constructs.


	FrameInfo nextFrame(PacketStreamSourceId src, SyncTime *sync);
	size_t readraw(char* target, size_t len);
	size_t skip(size_t len);

	bool good() const {return _stream.good();};

	size_t getPacketIndex(PacketStreamSourceId src_id) const; //returns the current frame for source
	inline size_t getNumPackets(PacketStreamSourceId src_id) const {return _index.packetCount(src_id);};

	FrameInfo seek(PacketStreamSourceId src, size_t framenum, SyncTime *sync = nullptr); //jumps to a particular packet.
	//If the address of a SyncTime is passed in, the object will be updated to maintain synchronization after the seek is complete.

//	void subscribe(PacketStreamSourceId);
//	void unsubscribe(PacketStreamSourceId);
//	FrameInfo nextFrame(SyncTime*);



    private:
	void init();
	void setupIndex();

	void parseHeader();
	void parseNewSource();
	void parseIndex();
	std::streampos parseFooter();

	FrameInfo _nextFrame();

	void waitForTimeSync(const SyncTime& timer, int64_t wait_for) const;


	void SkipSync();
	void ReSync() {_stream.syncToTag();};

};








}




