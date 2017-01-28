
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
	    json::value meta;
	    FrameInfo(): src(static_cast<decltype(src)>(-1)), time(-1), size(static_cast<decltype(size)>(-1)), sequence_num(static_cast<decltype(sequence_num)>(-1)){};
	    bool none(){return src == static_cast<decltype(src)>(-1);};
	};

    private:
	sourceIndexType _sources;
	std::map<size_t,size_t> _next_packet_framenum;

	packetIndex _index;

	std::ifstream _stream;
	bool _seekable;
	std::recursive_mutex _lock;

	int64_t _starttime;
//	using handlertype = size_t (*)(const FrameInfo&, iPacketStream&);
//	std::map<PacketStreamSourceId, handlertype> _handlers;
	pangoTagType _cachedtag;


    public:
	iPacketStream():_seekable(false), _starttime(0),_cachedtag(0){};
	iPacketStream(const std::string& sourcefile): _stream(sourcefile.c_str(), std::ios::in | std::ios::binary), _seekable(!IsPipe(sourcefile)), _starttime(0), _cachedtag(0){init();};
	void open(const std::string& sourcefile);
	void close();

	~iPacketStream(){close();};

	const decltype(_sources)& sources() const {return _sources;};

	//user is responsible for locking, since we cannot know the desired locking behaviour
	void lock() {_lock.lock();};
	void release() {_lock.unlock();};
	decltype(_lock)& mutex() {return _lock;}; //exposes the underlying mutex... this allows std::lock_guard, and similar constructs.

//	void setSrcHandler(PacketStreamSourceId src, handlertype handler){_handlers[src] = handler;};
//	//callback function must TAKE:
//	//const FrameInfo& (relevant data about the packet, including source id and size to read)
//	//iPacketStream& (a reference to *this, so the read can be performed)
//	//it must RETURN
//	//size_t (the TOTAL number of bytes the handler read from the stream).

	FrameInfo nextFrame(PacketStreamSourceId src, SyncTime *sync);
	FrameInfo nextFrame(SyncTime *sync); //from any source
	size_t readraw(char* target, size_t len);
	size_t skip(size_t len);

	bool good() const {return _stream.good();};

	size_t getPacketIndex(PacketStreamSourceId src_id) const; //returns the current frame for source

	inline size_t getNumPackets(PacketStreamSourceId src_id) const {return _index.packetCount(src_id);};


	FrameInfo seek(PacketStreamSourceId src, size_t framenum, SyncTime *sync = nullptr); //jumps to a particular packet. Throws std::runtime_error if the stream is not seekable.
	//may read other packets... so mixing this function with the packet handler callback is not recommended.
	//If the address of a SyncTime is passed in, the object will be updated to maintain synchronization after the seek is complete.


    private:
	pangoTagType readTag();
	pangoTagType peekTag();
	pangoTagType readTag(pangoTagType);

	void init();
	void setupIndex();

	void parseHeader();
	void parseNewSource();
	void parseIndex();
//	void parseSourceMeta();
	std::streampos parseFooter();


	FrameInfo _nextFrame();
	FrameInfo parsePacketHeader();

	void waitForTimeSync(const SyncTime& timer, int64_t wait_for) const;

	void seekg(std::streampos target){if (!_seekable) return; _cachedtag = 0; _stream.seekg(target);};
	void seekg (std::streamoff off, std::ios_base::seekdir way) {if (!_seekable) return; _cachedtag = 0; _stream.seekg(off, way);};
	std::streampos tellg() { return _cachedtag == 0 ? _stream.tellg() : _stream.tellg() - static_cast<std::streamoff>(TAG_LENGTH);};

	void SkipSync();
	void ReSync();

};








}




