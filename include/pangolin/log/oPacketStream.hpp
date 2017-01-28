
#pragma once

#include <ostream>

#include <pangolin/utils/threadedfilebuf.h>
#include <pangolin/utils/file_utils.h>
#include <pangolin/log/PacketStream.hpp>

namespace pangolin
{

class PANGOLIN_EXPORT oPacketStream
{
    private:
	threadedfilebuf _buffer;
	std::ostream _stream;
	bool _indexable, _open;

	sourceIndexType _sources;
	packetIndex _index;
	size_t _bytes_written;

    public:
	oPacketStream():_stream(&_buffer),_indexable(false),_open(false),_bytes_written(0){_stream.exceptions(std::ostream::badbit);};
	oPacketStream(const std::string& filename, size_t buffer_size  = 100*1024*1024):_buffer(pangolin::PathExpand(filename), buffer_size), _stream(&_buffer), _indexable(!IsPipe(filename)), _open(_stream.good()),_bytes_written(0)
	{
	    _stream.exceptions(std::ostream::badbit);
	    writeHeader();
	};

	void close() //does not write footer or index.
	{
	    if (_open)
	    {
		_buffer.close();
		_open = false;
	    }
	    _sources.clear();
	};
	void forceClose()
	{
	    if (_open)
	    {
		_buffer.force_close();
		close();
	    }
	}

	~oPacketStream(){close();};

	void open(const std::string& filename, size_t buffer_size = 100 * 1024 * 1024)
	{
	    close();
	    _buffer.open(filename, buffer_size);
	    _open = _stream.good();
	    _bytes_written = 0;
	    writeHeader();
	}


	PacketStreamSourceId addSource(PacketStreamSource& source); //writes to the stream immediately upon add. Return source id # and writes source id # to argument struct
	PacketStreamSourceId addSource(const PacketStreamSource& source); //if constructor is called inline
	void writePacket(PacketStreamSourceId src, const unsigned char* source, size_t sourcelen, const json::value& meta = json::value()) {writePacket(src, reinterpret_cast<const char*>(source), sourcelen, meta);};
	void writePacket(PacketStreamSourceId src, const char* source, size_t sourcelen, const json::value& meta = json::value() );
	void writeSync(); //for stream read/write synchronization. Note that this is NOT the same as time synchronization on playback of iPacketStreams.
	void writeEnd(); //writes the end of the stream data, including the index. Does NOT close the underlying ostream.

	const decltype(_sources)& sources() const {return _sources;};
	bool isOpen() const {return _open;};

//    private:
	void writeHeader();
	void write(PacketStreamSourceId, const PacketStreamSource&);
	void writeMeta(PacketStreamSourceId src, const json::value& data);
};


}

#undef DEBUG_CHECKPOINT
