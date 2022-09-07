//
// Created by 17271 on 2022/6/8.
//

#ifndef NETPP_BUFFERIOCONVERSION_H
#define NETPP_BUFFERIOCONVERSION_H

#include <cstddef>
#include "ByteArray.h"

struct iovec;
struct msghdr;
struct sockaddr_in;

namespace netpp {
class Datagram;
/**
 * @brief Gather ByteArray nodes together for read/write
 */
class ByteArrayGather {
public:
	ByteArrayGather();
	virtual ~ByteArrayGather();

	virtual void adjustByteArray(ByteArray::LengthType size) = 0;

	virtual ByteArray::LengthType availableBytes() = 0;

	::msghdr *msghdr() { return m_msghdr; }

	ByteArrayGather(ByteArrayGather &) = delete;
	ByteArrayGather(ByteArrayGather &&) = delete;
	ByteArrayGather &operator=(ByteArrayGather &) = delete;
	ByteArrayGather &operator=(ByteArrayGather &&) = delete;

protected:
	::msghdr *m_msghdr;
};

/**
 * @brief Convert a ByteArray to iovec, for read data from iovec
 * @note ByteArray's lock is acquired until destruction
 *
 */
class ByteArrayReaderWithLock : public ByteArrayGather {
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
class SequentialByteArrayReaderWithLock : public ByteArrayGather {
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
class ByteArrayWriterWithLock : public ByteArrayGather {
public:
	explicit ByteArrayWriterWithLock(std::shared_ptr<ByteArray> buffer);

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

extern ::sockaddr_in *dataGramDestinationExtractor(Datagram *data);

class DatagramReaderWithLock : public ByteArrayGather {
public:
	explicit DatagramReaderWithLock(std::shared_ptr<Datagram> data);
	~DatagramReaderWithLock() override;

	void adjustByteArray(ByteArray::LengthType size) override;
	ByteArray::LengthType availableBytes() override;

private:
	ByteArrayReaderWithLock m_reader;
};

class DatagramWriterWithLock : public ByteArrayGather {
public:
	explicit DatagramWriterWithLock(std::shared_ptr<Datagram> data);
	~DatagramWriterWithLock() override;

	void adjustByteArray(ByteArray::LengthType size) override;
	ByteArray::LengthType availableBytes() override;

private:
	ByteArrayWriterWithLock m_writer;
};
}

#endif //NETPP_BUFFERIOCONVERSION_H
