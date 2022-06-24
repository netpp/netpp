//
// Created by 17271 on 2022/6/8.
//

#ifndef NETPP_BUFFERIOCONVERSION_H
#define NETPP_BUFFERIOCONVERSION_H

#include <cstddef>
#include "ByteArray.h"

struct iovec;

namespace netpp {
/**
 * @brief Convert ByteArray to ::iovec, adapt to readv()/writev(), used by SocketIO
 */
class BufferIOConversion {
public:
	BufferIOConversion();

	virtual ~BufferIOConversion();

	::iovec *iovec()
	{ return m_vec; }

	[[nodiscard]] std::size_t iovenLength() const
	{ return m_vecLen; }

	virtual void adjustByteArray(ByteArray::LengthType size) = 0;

	virtual ByteArray::LengthType availableBytes() = 0;

	BufferIOConversion(BufferIOConversion &) = delete;

	BufferIOConversion(BufferIOConversion &&) = delete;

	BufferIOConversion &operator=(BufferIOConversion &) = delete;

	BufferIOConversion &operator=(BufferIOConversion &&) = delete;

protected:
	::iovec *m_vec;
	std::size_t m_vecLen;
};

/**
 * @brief Convert a ByteArray to iovec, for read data from iovec
 * @note ByteArray's lock is acquired until destruction
 *
 */
class ByteArrayReaderWithLock : public BufferIOConversion {
public:
	explicit ByteArrayReaderWithLock(std::shared_ptr <ByteArray> buffer);

	~ByteArrayReaderWithLock() override;

	/**
	 * @brief After read 'size' data from ByteArray, move read node backwards
	 * @param size The size expect to move
	 */
	void adjustByteArray(ByteArray::LengthType size) override;

	/**
	 * @brief Get readable bytes in ByteArray
	 *
	 * @return ByteArray::LengthType	readable bytes
	 */
	ByteArray::LengthType availableBytes() override;

private:
	std::shared_ptr <ByteArray> m_buffer;
	std::lock_guard<decltype(ByteArray::m_bufferMutex)> m_lck;
};

/**
 * @brief Convert several ByteArrays to one iovec, improve the flexibility of reading,
 * for example, we can prepend/append anything on an existent buffer, just create an
 * other ByteArray
 */
class SequentialByteArrayReaderWithLock : public BufferIOConversion {
public:
	SequentialByteArrayReaderWithLock(std::initializer_list <std::shared_ptr<ByteArray>> buffers);

	explicit SequentialByteArrayReaderWithLock(std::vector <std::shared_ptr<ByteArray>> &&buffers);

	~SequentialByteArrayReaderWithLock() override;

	/**
	 * @brief After read 'size' data from ByteArray, move read node backwards
	 * @param size The size expect to move
	 */
	void adjustByteArray(ByteArray::LengthType size) override;

	/**
	 * @brief Get readable bytes in ByteArrays
	 *
	 * @return ByteArray::LengthType	readable bytes
	 */
	ByteArray::LengthType availableBytes() override;

private:
	using ByteArrayMutex =
	decltype(ByteArray::m_bufferMutex);
	std::vector <std::shared_ptr<ByteArray>> m_buffers;
	std::vector <std::unique_ptr<std::lock_guard < ByteArrayMutex>>>
	m_lck;
};

/**
 * @brief Convert ByteArray to iovec, for write data to iovec
 * @note ByteArray's lock is acquired until destruction
 *
 */
class ByteArrayWriterWithLock : public BufferIOConversion {
public:
	explicit ByteArrayWriterWithLock(std::shared_ptr <ByteArray> buffer);

	~ByteArrayWriterWithLock() override;

	/**
	 * @brief After write 'size' data to ByteArray, move write node backwards
	 * @param size The size expect to move
	 */
	void adjustByteArray(ByteArray::LengthType size) override;

	/**
	 * @brief Get writeable bytes in ByteArray
	 *
	 * @return ByteArray::LengthType	writeable bytes
	 */
	ByteArray::LengthType availableBytes() override;

private:
	std::shared_ptr <ByteArray> m_buffer;
	std::lock_guard<decltype(ByteArray::m_bufferMutex)> m_lck;
};
}

#endif //NETPP_BUFFERIOCONVERSION_H
