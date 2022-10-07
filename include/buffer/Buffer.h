//
// Created by gaojian on 2022/9/27.
//

#ifndef NETPP_BUFFER_H
#define NETPP_BUFFER_H

#include <cstddef>
#include "support/Types.h"

namespace netpp {
class Buffer {
public:
	/**
	 * @brief Default ByteArray
	 *
	 * By default, alloc
	 */
	Buffer();

	/**
	 * @brief Copy from other ByteArray is COW(copy on write)
	 * @param other an other ByteArray
	 */
	Buffer(const Buffer &other);

	/**
	 * @brief Move construct a ByteArray from other
	 * @param other an other ByteArray
	 */
	Buffer(Buffer &&other) noexcept;

	~Buffer();

	/**
	 * @brief Write raw data to ByteArray
	 * @param data The raw data
	 * @param length The size of data
	 */
	void write(const char *data, BufferLength length);

	/**
	 * @brief Retrieve raw data from ByteArray to buffer
	 * @param buffer		Store data here
	 * @param length		How many bytes to retrieve
	 * @return std::size_t	The actual size retrieved from ByteArray
	 */
	BufferLength retrieve(char *buffer, BufferLength length);

	/**
	 * @brief The readable bytes in buffer. From ReadNode's start to WriteNode's end.
	 */
	[[nodiscard]] BufferLength readableBytes() const;

	/**
	 * @brief The writeable bytes in buffer.
	 */
	[[nodiscard]] BufferLength writeableBytes() const;

	char *getRawPointer();

	void readOutSize(BufferLength size);
	void writeInSize(BufferLength size);

private:
	/**
	 * @brief The buffer node size
	 */
	static constexpr BufferLength DefaultBufferSize = 1024;

	void allocAtLeast(BufferLength size);

	BufferLength m_bufferSize;
	BufferLength m_start;
	BufferLength m_end;
	char *m_buffer;
};
} // netpp

#endif //NETPP_BUFFER_H
