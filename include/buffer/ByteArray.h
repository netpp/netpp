#ifndef NETPP_BYTEARRAY_H
#define NETPP_BYTEARRAY_H

#include <string>
#include <memory>
#include <mutex>
#include <vector>
#include "BufferNode.h"
#include "CowLink.hpp"

namespace netpp {
class ByteArrayReaderWithLock;
class ByteArrayWriterWithLock;
class SequentialByteArrayReaderWithLock;
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
 * @htmlonly
 * <pre>
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
 * @endhtmlonly
 */
class ByteArray {
	/**
	 * @brief Convert buffer to iovec, read buffer
	 * @note ByteArray's public methods are guarded by mutex, do NOT use with ByteArrayIOVector*WithLock, it will lead to dead lock
	 */
	friend class ByteArrayReaderWithLock;
	/**
	 * @brief Convert buffer to iovec, write to buffer
	 * @note ByteArray's public methods are guarded by mutex, do NOT use with ByteArrayIOVector*WithLock, it will lead to dead lock
	 */
	friend class ByteArrayWriterWithLock;
	/**
	 * @brief Convert buffer to iovec, read several buffers
	 * @note ByteArray's public methods are guarded by mutex, do NOT use with ByteArrayIOVector*WithLock, it will lead to dead lock
	 */
	friend class SequentialByteArrayReaderWithLock;
public:
	/**
	 * @brief The buffer node size type
	 */
	using LengthType = BufferNode::LengthType;
	/**
	 * @brief The buffer node size
	 */
	static constexpr LengthType BufferNodeSize = BufferNode::BufferNodeSize;

	/**
	 * @brief Default ByteArray
	 *
	 * By default, alloc
	 */
	ByteArray();

	/**
	 * @brief Copy from other ByteArray is COW(copy on write)
	 * @param other an other ByteArray
	 */
	ByteArray(const ByteArray &other);

	/**
	 * @brief Move construct a ByteArray from other
	 * @param other an other ByteArray
	 */
	ByteArray(ByteArray &&other) noexcept;

	/**
	 * @brief Write an int8 value to ByteArray
	 * @param value The value to write
	 */
	void writeInt8(int8_t value);
	/**
	 * @brief Write an int16 value to ByteArray, the value will be stored in network-ending
	 * @param value The value to write
	 */
	void writeInt16(int16_t value);
	/**
	 * @brief Write an int32 value to ByteArray, the value will be stored in network-ending
	 * @param value The value to write
	 */
	void writeInt32(int32_t value);
	/**
	 * @brief Write an int64 value to ByteArray, the value will be stored in network-ending
	 * @param value The value to write
	 */
	void writeInt64(int64_t value);
	/**
	 * @brief Write a uint8 value to ByteArray
	 * @param value The value to write
	 */
	void writeUInt8(uint8_t value);
	/**
	 * @brief Write a uint16 value to ByteArray, the value will be stored in network-ending
	 * @param value The value to write
	 */
	void writeUInt16(uint16_t value);
	/**
	 * @brief Write a uint32 value to ByteArray, the value will be stored in network-ending
	 * @param value The value to write
	 */
	void writeUInt32(uint32_t value);
	/**
	 * @brief Write a uint64 value to ByteArray, the value will be stored in network-ending
	 * @param value The value to write
	 */
	void writeUInt64(uint64_t value);
	/**
	 * @brief Write a float value to ByteArray
	 * @note byte order not converted
	 * @param value The value to write
	 */
	void writeFloat(float value);
	/**
	 * @brief Write a float value to ByteArray
	 * @note byte order not converted
	 * @param value The value to write
	 */
	void writeDouble(double value);
	/**
	 * @brief Write a string to ByteArray
	 * @param value The value to write
	 */
	void writeString(const std::string &value);
	/**
	 * @brief Write raw data to ByteArray
	 * @param data The raw data
	 * @param length The size of data
	 */
	void writeRaw(const char *data, std::size_t length);

	// TODO: support read until meet certain value

	/**
	 * @brief Read 1 byte, convert it to int8
	 * @return 0 if readable size less than 1 byte
	 * @return otherwise return the value force convert to int8
	 */
	int8_t retrieveInt8();
	/**
	 * @brief Read 2 bytes, convert it to int16
	 * @return 0 if readable size less than 2 bytes
	 * @return otherwise read 2 bytes, convert network-ending to local-ending, cast int16
	 */
	int16_t retrieveInt16();
	/**
	 * @brief Read 4 bytes, convert it to int32
	 * @return 0 if readable size less than 4 bytes
	 * @return otherwise read 4 bytes, convert network-ending to local-ending, cast int32
	 */
	int32_t retrieveInt32();
	/**
	 * @brief Read 8 bytes, convert it to int64
	 * @return 0 if readable size less than 8 byte
	 * @return otherwise read 8 bytes, convert network-ending to local-ending, cast int64
	 */
	int64_t retrieveInt64();
	/**
	 * @brief Read 1 byte, convert it to uint8
	 * @return 0 if readable size less than 1 byte
	 * @return otherwise return the value force convert to uint8
	 */
	uint8_t retrieveUInt8();
	/**
	 * @brief Read 2 bytes, convert it to uint16
	 * @return 0 if readable size less than 2 bytes
	 * @return otherwise read 2 bytes, convert network-ending to local-ending, cast uint16
	 */
	uint16_t retrieveUInt16();
	/**
	 * @brief Read 4 bytes, convert it to uint32
	 * @return 0 if readable size less than 4 bytes
	 * @return otherwise read 4 bytes, convert network-ending to local-ending, cast uint32
	 */
	uint32_t retrieveUInt32();
	/**
	 * @brief Read 8 bytes, convert it to uint64
	 * @return 0 if readable size less than 8 bytes
	 * @return otherwise read 8 bytes, convert network-ending to local-ending, cast uint64
	 */
	uint64_t retrieveUInt64();
	/**
	 * @brief Read 4 bytes, convert it to float
	 * @return 0 if readable size less than 4 bytes
	 * @return otherwise read 4 bytes, not ending convert performed, cast float
	 */
	float retrieveFloat();
	/**
	 * @brief Read 8 bytes, convert it to double
	 * @return 0 if readable size less than 8 bytes
	 * @return otherwise read 8 bytes, not ending convert performed, cast double
	 */
	double retrieveDouble();
	/**
	 * @brief Read a string
	 * @param length The bytes to read
	 * @return A string
	 */
	std::string retrieveString(std::size_t length);
	/**
	 * @brief Retrieve raw data from ByteArray to buffer
	 * @param buffer		Store data here
	 * @param length		How many bytes to retrieve
	 * @return std::size_t	The actual size retrieved from ByteArray
	 */
	std::size_t retrieveRaw(char *buffer, std::size_t length);

	/**
	 * @brief The readable bytes in buffer. From ReadNode's start to WriteNode's end.
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
	 * @htmlonly
	 * <pre>
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
	 * @endhtmlonly
	 */
	LengthType writeableBytes() const
	{
		std::lock_guard lck(m_bufferMutex);
		return m_availableSizeToWrite;
	}

private:
	using CowBuffer = CowLink<BufferNode>;

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
