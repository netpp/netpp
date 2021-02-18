#ifndef NETPP_BYTEARRAY_H
#define NETPP_BYTEARRAY_H

#include <cstddef>
#include <string>
#include <memory>
#include <mutex>
#include <atomic>

namespace netpp {
namespace internal::socket {
class ByteArrayIOVectorReaderWithLock;
class ByteArrayIOVectorWriterWithLock;
}
/**
 * @brief Save bytes in network ending(big ending)
 */
class ByteArray {
	// access buffer directly
	friend class internal::socket::ByteArrayIOVectorReaderWithLock;
	friend class internal::socket::ByteArrayIOVectorWriterWithLock;
public:
	ByteArray();

	void writeInt8(int8_t value);
	void writeInt16(int16_t value);
	void writeInt32(int32_t value);
	void writeInt64(int64_t value);
	void writeUInt8(uint8_t value);
	void writeUInt16(uint16_t value);
	void writeUInt32(uint32_t value);
	void writeUInt64(uint64_t value);
	void writeFloat(float value);
	void writeDouble(double value);
	void writeString(std::string value);
	void writeRaw(const char *data, std::size_t length);

	int8_t retrieveInt8();
	int16_t retrieveInt16();
	int32_t retrieveInt32();
	int64_t retrieveInt64();
	uint8_t retrieveUInt8();
	uint16_t retrieveUInt16();
	uint32_t retrieveUInt32();
	uint64_t retrieveUInt64();
	float retrieveFloat();
	double retrieveDouble();
	std::string retrieveString(std::size_t length);
	std::size_t retrieveRaw(char *buffer, std::size_t length);

	/**
	 * @brief The readable bytes in buffer
	 */
	inline std::size_t readableBytes() { return m_availableSizeToRead; }

private:
	/**
	 * @brief Alloc buffer
	 * 1. move unused buffer node from head to tail
	 * 2. if available writeable buffer size is still less than size, alloc double node than current node count
	 * @param size at lease n bytes available
	 */
	void allocIfNotEnough(std::size_t size);

	/**
	 * @brief Move unused buffer to tail, avoid memory alloc
	 */
	void moveBufferHead();

	/**
	 * @brief The node of buffer
	 */
	// FIXME: remove unused buffer node
	struct BufferNode {
		BufferNode();
		constexpr static std::size_t BufferSize = 1024;
		// constexpr static int maxTimeToLive = 10;
		std::size_t start;	// the offset of buffer read
		std::size_t end;	// the offset of buffer write
		// int timeToLive;
		char buffer[BufferSize];	// buffer
		std::shared_ptr<BufferNode> next;	// next buffer node
	};
	std::mutex m_bufferMutex;
	std::atomic_uint64_t m_availableSizeToRead;
	uint64_t m_availableSizeToWrite;
	unsigned m_nodeCount;						// node number
	std::shared_ptr<BufferNode> m_bufferHead;	// the head of buffer node
	std::weak_ptr<BufferNode> _bufferTail;		// the tail of buffer node
	std::weak_ptr<BufferNode> _currentReadBufferNode;	// start read buffer from
	std::weak_ptr<BufferNode> _currentWriteBufferNode;	// start write buffer from
};
}

#endif
