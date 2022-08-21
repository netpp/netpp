//
// Created by gaojian on 2022/3/22.
//

#ifndef NETPP_TCPBUFFER_H
#define NETPP_TCPBUFFER_H

#include "Buffer.h"
#include <vector>

namespace netpp {
class ByteArray;
class BufferIOConversion;
/**
 * @brief The normal tcp connection buffer
 */
class TcpBuffer : public Buffer {
public:
	~TcpBuffer() override;

	void addWriteBuffer(const ByteArray &buffer) override;

	ByteArray peekReadBuffer(ByteArray::LengthType size) override;
	ByteArray readBuffer(ByteArray::LengthType size) override;

	/**
	* @brief Read from buffer, and send to peer
	* @return The ByteArray to iovec convertor
	*/
	std::unique_ptr<BufferIOConversion> sendBufferForIO() override;

	/**
	* @brief Write peer message to buffer
	* @return The ByteArray to iovec convertor
	*/
	std::unique_ptr<BufferIOConversion> receiveBufferForIO() override;

	ByteArray::LengthType readableBytes() const override;
private:
	std::vector<std::shared_ptr<ByteArray>> m_sendBuffers;
	std::shared_ptr<ByteArray> m_receiveArray;
};
}

#endif //NETPP_TCPBUFFER_H
