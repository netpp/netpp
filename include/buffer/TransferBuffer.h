//
// Created by gaojian on 2020/7/9.
//

#ifndef NETPP_TRANSFERBUFFER_H
#define NETPP_TRANSFERBUFFER_H

#include <memory>
#include "buffer/ByteArray.h"

namespace netpp {
class ByteArrayGather;
class ByteArray;
/**
 * @brief A Channel is an implement of SocketConnectionHandler's buffer.
 *
 * For user, Channel must provide a way to access buffer, data can be read/write into buffer
 * For tcp connection, Channel is buffer.
 */
class TransferBuffer {
public:
	virtual ~TransferBuffer() = default;

	virtual void write(const ByteArray &buffer) = 0;
	virtual ByteArray peek(ByteArray::LengthType size) = 0;
	virtual ByteArray read(ByteArray::LengthType size) = 0;

	/**
	 * @brief A Channel is for user, should not expose implementation of low level io,
	 * ChannelBufferConversion is support class to do the transformation. Each Channel's
	 * subclass should implement it's ChannelBufferConversion object.
	 * @return The specified ChannelBufferConversion for Channel
	 */
	virtual std::unique_ptr<ByteArrayGather> sendBufferForIO() = 0;
	virtual std::unique_ptr<ByteArrayGather> receiveBufferForIO() = 0;

	[[nodiscard]] virtual ByteArray::LengthType bytesReceived() const = 0;
	[[nodiscard]] virtual ByteArray::LengthType bytesCanBeSend() const = 0;
};
}

#endif //NETPP_TRANSFERBUFFER_H
