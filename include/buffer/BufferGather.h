//
// Created by gaojian on 2022/6/8.
//

#ifndef NETPP_BUFFERGATHER_H
#define NETPP_BUFFERGATHER_H

#include <cstddef>
#include "support/Types.h"
#include <queue>
#include <map>

struct iovec;
struct msghdr;
struct sockaddr_in;

namespace netpp {
class Buffer;
class Address;
/**
 * @brief Unit and manage buffers
 */
class BufferGather {
public:
	BufferGather();
	virtual ~BufferGather();

	virtual void adjustByteArray(BufferLength size) = 0;

	virtual BufferLength availableBytes() = 0;

	virtual std::shared_ptr<Buffer> getBuffer() = 0;

	::msghdr *msghdr() { return m_msghdr; }

	BufferGather(BufferGather &) = delete;
	BufferGather(BufferGather &&) = delete;
	BufferGather &operator=(BufferGather &) = delete;
	BufferGather &operator=(BufferGather &&) = delete;

protected:
	::msghdr *m_msghdr;
};

class MultipleBufferNodesGather {
public:
	virtual ~MultipleBufferNodesGather() = default;

	virtual void addBufferNode(const std::shared_ptr<Buffer> &buffer) = 0;
};
/**
 * @brief An united buffer for read
 *
 */
class SingleBufferReadGather : public BufferGather {
public:
	SingleBufferReadGather();

	~SingleBufferReadGather() override;

	/**
	 * @brief After read 'size' data from ByteArray, move read node backwards
	 * @param size The size expect to move
	 */
	void adjustByteArray(BufferLength size) override;

	/**
	 * @brief Get readable bytes in ByteArray
	 *
	 * @return ByteArray::LengthType	readable bytes
	 */
	BufferLength availableBytes() override;

	std::shared_ptr<Buffer> getBuffer() override { return nullptr; }

private:
	std::shared_ptr<Buffer> m_buffer;
};

/**
 * @brief Convert several ByteArrays to one iovec, improve the flexibility of reading,
 * for example, we can prepend/append anything on an existent buffer, just create an
 * other ByteArray
 */
class SequentialBufferReadGather : public BufferGather, public MultipleBufferNodesGather {
public:
	SequentialBufferReadGather();
	~SequentialBufferReadGather() override;

	/**
	 * @brief After read 'size' data from ByteArray, move read node backwards
	 * @param size The size expect to move
	 */
	void adjustByteArray(BufferLength size) override;

	/**
	 * @brief Get readable bytes in ByteArrays
	 *
	 * @return ByteArray::LengthType	readable bytes
	 */
	BufferLength availableBytes() override;

	std::shared_ptr<Buffer> getBuffer() override;

	void addBufferNode(const std::shared_ptr<Buffer> &buffer) override;

private:
	std::vector<std::shared_ptr<Buffer>> m_buffers;
};

/**
 * @brief Convert ByteArray to iovec, for write data to iovec
 * @note ByteArray's lock is acquired until destruction
 *
 */
class BufferWriteGather : public BufferGather {
public:
	BufferWriteGather();

	~BufferWriteGather() override;

	/**
	 * @brief After write 'size' data to ByteArray, move write node backwards
	 * @param size The size expect to move
	 */
	void adjustByteArray(BufferLength size) override;

	/**
	 * @brief Get writeable bytes in ByteArray
	 *
	 * @return ByteArray::LengthType	writeable bytes
	 */
	BufferLength availableBytes() override;

	std::shared_ptr<Buffer> getBuffer() override;

private:
	static constexpr BufferLength extraBufferSize = 10240;
	std::shared_ptr<Buffer> m_buffer;
};

/*class DatagramReadGather : public BufferGather, public MultipleBufferNodesGather {
public:
	DatagramReadGather();
	~DatagramReadGather() override;

	void adjustByteArray(BufferLength size) override;
	BufferLength availableBytes() override;
	void addBufferNode(const std::shared_ptr<Buffer> &buffer) override;
	void addBufferNode(const std::shared_ptr<Buffer> &buffer, const Address &destination);
private:
	void addBufferNode(const std::shared_ptr<Buffer> &buffer, ::sockaddr_in *address);
	struct DGData {
		std::shared_ptr<Buffer> buffer;
		::msghdr *msg;
	};
	std::queue<void *> m_bufferQueue;
	std::map<void *, DGData> m_pendingSend;
};

class DatagramWriteGather : public BufferGather {
public:
	explicit DatagramWriteGather();
	~DatagramWriteGather() override;

	void adjustByteArray(BufferLength size) override;
	BufferLength availableBytes() override;

private:

};*/

class ByteArray;
class Datagram;
//extern ::sockaddr_in *extractDestination(const Datagram *data);
extern std::shared_ptr<Buffer> extractBuffer(const ByteArray *byteArray);
}

#endif //NETPP_BUFFERGATHER_H
