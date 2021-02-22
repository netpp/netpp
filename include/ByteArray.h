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
 * @brief ByteArray has a list of buffer nodes, saved in network ending(big ending), 
 * it's thread safe.
 * 
 * The ReadNode point to node where data begins, while reading, the node's start 
 * indicator moves, if reached the end indicator, all data in this node has read, 
 * ReadNode will try to move to next node(if any), the movement of ReadNode makes
 * nodes between head and ReadNode out of usage, these nodes will be moved later.
 * ReadNode should never cross WriteNode.
 * 
 * The WriteNode point to node where data ends(also where to start write new data),
 * writing to a ByteArray will move node's end indicator, if reached the max size
 * of node's buffer, WriteNode will to move to next node.
 * Before any data wrote, the writeable bytes will be checked, if the pending length
 * is not satisfied, the head movement or memory allocation will applied.
 * The 'head movement' target at move 'unused' nodes(as I said before, the movement 
 * of ReadNode, lead nodes between head and ReadNode out of usage) to tail, avoid 
 * memory allocation.
 * If still not enough space for pending data after move, allocat nodes twice than 
 * current every time utill data can be stored. 
 * 
 *               ReadNode   WriteNode
 *                  |           |
 * +-------+    +-------+    +-------+    +-------+
 * | Node1 | -> | Node2 | -> | Node3 | -> | Node4 | -> nullptr
 * +-------+    +-------+    +-------+    +-------+
 *      |           |            |            |
 * +----------++----------++----------++----------+
 * |  buffer  ||  buffer  ||  buffer  ||  buffer  |
 * +----------++----------++----------++----------+
 *                  |    |  |     |
 *     	         start/end  start/end
 * 
 */
class ByteArray {
	// access buffer directly
	/// @note public methods are guarded by mutex, do NOT use with ByteArrayIOVector*WithLock, it will lead to dead lock
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
	/**
	 * @brief retrive raw data from ByteArray to buffer
	 * 
	 * @param buffer		store data here
	 * @param length		how many bytes to retrieve
	 * @return std::size_t	the actual size retrieved from ByteArray
	 */
	std::size_t retrieveRaw(char *buffer, std::size_t length);

	/**
	 * @brief The readable bytes in buffer.
	 * From ReadNode's start to WriteNode's endl.
	 */
	inline std::uint64_t readableBytes() { std::lock_guard lck(m_bufferMutex); return m_availableSizeToRead; }

	/** 
	 * @brief The unused bytes in buffer.
	 * The 'unused' means the length from WriteNode's end to last node's end.
	 * 
	 * WriteNode
	 *     |
	 * +-------+    +-------+
	 * | Node3 | -> | Node4 | -> nullptr
	 * +-------+    +-------+
	 *     |            |
	 * +----------++----------+
	 * |  buffer  ||  buffer  |
	 * +----------++----------+
	 *    end|                |
	 *       |-----unused-----|
	 */
	inline std::uint64_t unusedBytes() { std::lock_guard lck(m_bufferMutex); return m_availableSizeToWrite; }

private:
	/**
	 * @brief Alloc more buffer
	 * 1. move unused buffer node from head to tail
	 * 2. if available writeable buffer size is still less than size, alloc double size of nodes than current
	 * @param size	at lease n bytes available
	 * @note this method will not acquire lock
	 */
	void unlockedAllocIfNotEnough(std::size_t size);

	/**
	 * @brief Move unused buffer to tail, avoid memory alloc
	 * @note this method will not acquire lock
	 */
	void unlockedMoveBufferHead();

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
	std::uint64_t m_availableSizeToRead;
	std::uint64_t m_availableSizeToWrite;
	unsigned m_nodeCount;						// node number
	std::shared_ptr<BufferNode> m_bufferHead;	// the head of buffer node
	std::weak_ptr<BufferNode> _bufferTail;		// the tail of buffer node
	std::weak_ptr<BufferNode> _currentReadBufferNode;	// start read buffer from
	std::weak_ptr<BufferNode> _currentWriteBufferNode;	// start write buffer from
};
}

#endif
