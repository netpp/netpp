//
// Created by gaojian on 2020/7/9.
//

#ifndef NETPP_BUFFER_H
#define NETPP_BUFFER_H

#include <memory>

namespace netpp {
class BufferIOConversion;
class ByteArray;
/**
 * @brief A Channel is an implement of SocketConnectionHandler's buffer.
 *
 * For user, Channel must provide a way to access buffer, data can be read/write into buffer
 * For tcp connection, Channel is buffer.
 */
class Buffer {
public:
	virtual ~Buffer() = default;
	virtual void addWriteBuffer(std::shared_ptr<ByteArray> buffer) = 0;
	virtual std::unique_ptr<BufferIOConversion> sendBufferForIO() = 0;
	virtual std::unique_ptr<BufferIOConversion> receiveBufferForIO() = 0;
};
}

#endif //NETPP_BUFFER_H
