#ifndef NETPP_BYTEARRAY_H
#define NETPP_BYTEARRAY_H

#include <string>
#include <memory>
#include <mutex>
#include <vector>
#include "internal/buffer/BufferNode.h"
#include "internal/buffer/CowLink.hpp"

namespace netpp {
namespace internal::socket {
class ByteArrayReaderWithLock;
class ByteArrayWriterWithLock;
class SequentialByteArrayReaderWithLock;
}
/**
 * @brief ByteArray has a list of buffer nodes, stored in network ending(big ending),
 * it's thread safe.
 * 
 * @section ReadNode
 * The ReadNode point to node where data begins, while reading, the node's start 
 * indicator moves, if reached the end indicator, all data in this node has read, 
 * ReadNode will try to move to next node(if any), the movement of ReadNode makes
 * m_nodes between head and ReadNode out of usage, these m_nodes will be moved later.
 * ReadNode should never cross WriteNode.
 * 
 * @section WriteNode
 * The WriteNode point to node where data ends(also where to start write new data),
 * writing to a ByteArray will move node's end indicator, if reached the max size
 * of node's buffer, WriteNode will to move to next node.
 * Before any data wrote, the writeable bytes will be checked, if the pending length
 * is not satisfied, the head movement or memory allocation will applied.
 * The 'head movement' target at move 'writeable' m_nodes(as I said before, the movement
 * of ReadNode, lead m_nodes between head and ReadNode out of usage) to tail, avoid
 * memory allocation.
 * If still not enough space for pending data after move, allocate m_nodes twice than
 * current every time util data can be stored.
 * 
 * @section node graphic
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
 * </pre>
 */
class ByteArray {
	/**
	 * @brief access buffer directly
	 * @note public methods are guarded by mutex, do NOT use with ByteArrayIOVector*WithLock, it will lead to dead lock
	 */
	friend class internal::socket::ByteArrayReaderWithLock;
	friend class internal::socket::ByteArrayWriterWithLock;
	friend class internal::socket::SequentialByteArrayReaderWithLock;
public:
	using LengthType = internal::buffer::BufferNode::LengthType;
	static constexpr LengthType BufferNodeSize = internal::buffer::BufferNode::BufferNodeSize;

	ByteArray();
	/**
	 * Copy from other ByteArray is COW(copy on write)
	 * @param other an other ByteArray
	 */
	ByteArray(const ByteArray &other);
	ByteArray(ByteArray &&other) noexcept;

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
	void writeString(const std::string &value);
	void writeRaw(const char *data, std::size_t length);

	// TODO: support read until meet certain value

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
	 * @brief Retrieve raw data from ByteArray to buffer
	 * 
	 * @param buffer		Store data here
	 * @param length		How many bytes to retrieve
	 * @return std::size_t	The actual size retrieved from ByteArray
	 */
	std::size_t retrieveRaw(char *buffer, std::size_t length);

	/**
	 * @brief The readable bytes in buffer.
	 * From ReadNode's start to WriteNode's end.
	 */
	LengthType readableBytes() const
	{
		std::lock_guard lck(m_bufferMutex);
		return m_availableSizeToRead;
	}

	/** 
	 * @brief The writeable bytes in buffer.
	 * The 'writeable' means the length from WriteNode's end to last node's end,
	 * the ByteArray length will be automatically increased, if can not store pending data.
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
	 *       |----writeable---|
	 */
	LengthType writeableBytes() const
	{
		std::lock_guard lck(m_bufferMutex);
		return m_availableSizeToWrite;
	}

private:
	using CowBuffer = internal::buffer::CowLink<internal::buffer::BufferNode>;

	/**
	 * @brief Alloc more buffer
	 * 1. move unused buffer node from head to tail
	 * 2. if available writeable buffer size is still less than size, alloc double size of m_nodes than current
	 * @param size	At lease n bytes available
	 * @note This method will not acquire lock
	 */
	void unlockedAllocIfNotEnough(std::size_t size);

	/**
	 * @brief Move unused buffer to tail, avoid memory alloc
	 * @note This method will not acquire lock
	 */
	void unlockedMoveBufferHead();

	CowBuffer::NodeContainerIndexer endOfReadNode() const;

	mutable std::mutex m_bufferMutex;
	LengthType m_availableSizeToRead;
	LengthType m_availableSizeToWrite;

	CowBuffer::NodeContainerIndexer m_readNode;
	CowBuffer::NodeContainerIndexer m_writeNode;
	std::unique_ptr<CowBuffer> m_nodes;
};
}

#endif
