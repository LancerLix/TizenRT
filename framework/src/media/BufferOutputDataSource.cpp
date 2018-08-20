/* ****************************************************************
 *
 * Copyright 2018 Samsung Electronics All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ******************************************************************/

#include <tinyara/config.h>
#include <stdio.h>
#include <debug.h>
#include <media/BufferOutputDataSource.h>
#include "MediaRecorderImpl.h"
#include "OutputStreamBuffer.h"
#include "StreamBufferReader.h"

#ifndef CONFIG_BUFFER_DATASOURCE_STREAM_BUFFER_SIZE
#define CONFIG_BUFFER_DATASOURCE_STREAM_BUFFER_SIZE 4096
#endif

#ifndef CONFIG_BUFFER_DATASOURCE_STREAM_BUFFER_THRESHOLD
#define CONFIG_BUFFER_DATASOURCE_STREAM_BUFFER_THRESHOLD 1
#endif

namespace media {
namespace stream {
BufferOutputDataSource::BufferOutputDataSource()
	: OutputDataSource()
{
}

BufferOutputDataSource::BufferOutputDataSource(unsigned int channels, unsigned int sampleRate, audio_format_type_t pcmFormat)
	: OutputDataSource(channels, sampleRate, pcmFormat)
{
}

BufferOutputDataSource::BufferOutputDataSource(const BufferOutputDataSource& source)
	: OutputDataSource(source)
{
}

BufferOutputDataSource& BufferOutputDataSource::operator=(const BufferOutputDataSource& source)
{
	OutputDataSource::operator=(source);
	return *this;
}

bool BufferOutputDataSource::open()
{
	if (!getStreamBuffer()) {
		auto streamBuffer = OutputStreamBuffer::create( \
			CONFIG_BUFFER_DATASOURCE_STREAM_BUFFER_SIZE, \
			CONFIG_BUFFER_DATASOURCE_STREAM_BUFFER_THRESHOLD);

		if (!streamBuffer) {
			medvdbg("streamBuffer is nullptr!\n");
			return false;
		}

		setStreamBuffer(streamBuffer);
	}

	start();
	return true;
}

bool BufferOutputDataSource::close()
{
	stop();
	return true;
}

bool BufferOutputDataSource::isPrepare()
{
	return (getStreamBuffer() != nullptr);
}

ssize_t BufferOutputDataSource::onStreamBufferReadable(bool isFlush)
{
	auto recorder = getRecorder();
	if (recorder) {
		auto reader = getBufferReader();
		assert(reader->sizeOfData() > 0);
		size_t size = reader->sizeOfData();
		unsigned char *buffer = new unsigned char[size];
		if (buffer) {
			reader->read(buffer, size);
			recorder->notifyObserver(OBSERVER_COMMAND_BUFFER_DATAREACHED, buffer, size);
			delete[] buffer;
			return size;
		}

		meddbg("buffer alloc failed\n");
		return -1;
	}

	meddbg("Not register recorder, return 0\n");
	return 0;
}

BufferOutputDataSource::~BufferOutputDataSource()
{
	close();
}

} // namespace stream
} // namespace media
