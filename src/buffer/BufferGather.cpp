//
// Created by gaojian on 2022/6/8.
//

#include "buffer/BufferGather.h"
#include "location/Address.h"
#include "support/Util.h"
#include <cstring>
#include "buffer/Buffer.h"
#include "buffer/ByteArray.h"
extern "C" {
#include <sys/uio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
}

namespace netpp {
BufferGather::BufferGather()
{
	m_msghdr = new ::msghdr;
	std::memset(m_msghdr,0, sizeof(::msghdr));
}

BufferGather::~BufferGather()
{
	delete []m_msghdr->msg_iov;
	delete m_msghdr;
}

SingleBufferReadGather::SingleBufferReadGather()
		: m_buffer{std::make_shared<Buffer>()}
{
	auto *vecNodes = new ::iovec[2];
	vecNodes[0].iov_base = m_buffer->getRawPointer();
	vecNodes[0].iov_len = m_buffer->writeableBytes();
	m_msghdr->msg_iov = vecNodes;
	m_msghdr->msg_iovlen = 1;
}

SingleBufferReadGather::~SingleBufferReadGather() = default;

void SingleBufferReadGather::adjustByteArray(BufferLength size)
{
	if (size > m_buffer->readableBytes())
		m_buffer->readOutSize(m_buffer->readableBytes());
	else
		m_buffer->readOutSize(size);
	m_msghdr->msg_iov[0].iov_base = m_buffer->getRawPointer();
	m_msghdr->msg_iov[0].iov_len = m_buffer->writeableBytes();
}

BufferLength SingleBufferReadGather::availableBytes()
{
	return m_buffer->readableBytes();
}

BufferWriteGather::BufferWriteGather()
	: m_buffer{std::make_shared<Buffer>()}
{
	auto *vecNodes = new ::iovec[2];
	vecNodes[0].iov_base = m_buffer->getRawPointer();
	vecNodes[0].iov_len = m_buffer->writeableBytes();
	vecNodes[1].iov_base = new char[extraBufferSize];
	vecNodes[1].iov_len = extraBufferSize;
	m_msghdr->msg_iov = vecNodes;
	m_msghdr->msg_iovlen = 2;
}

BufferWriteGather::~BufferWriteGather() = default;

void BufferWriteGather::adjustByteArray(BufferLength size)
{
	BufferLength currentWritable = m_buffer->writeableBytes();
	if (size > currentWritable)
	{
		m_buffer->writeInSize(currentWritable);
		m_buffer->write(static_cast<char *>(m_msghdr->msg_iov[1].iov_base), size - currentWritable);
	}
	else
	{
		m_buffer->writeInSize(size);
	}
	m_msghdr->msg_iov[0].iov_base = m_buffer->getRawPointer();
	m_msghdr->msg_iov[0].iov_len = m_buffer->writeableBytes();
}

BufferLength BufferWriteGather::availableBytes()
{
	return m_buffer->writeableBytes();
}

std::shared_ptr<Buffer> BufferWriteGather::getBuffer()
{
	return m_buffer;
}

SequentialBufferReadGather::SequentialBufferReadGather()
{
	m_msghdr->msg_iov = nullptr;
	m_msghdr->msg_iovlen = 0;
}

SequentialBufferReadGather::~SequentialBufferReadGather() = default;

void SequentialBufferReadGather::adjustByteArray(BufferLength size)
{
	for (auto it = m_buffers.begin(); it != m_buffers.end();)
	{
		auto &buffer = *it;
		BufferLength readableSize = buffer->readableBytes();
		if (size >= readableSize)    // if size is larger than this ByteArray, just read node to end
		{
			buffer->readOutSize(readableSize);
			size -= readableSize;
			it = m_buffers.erase(it);
		}
		else
		{
			buffer->readOutSize(size);
			size = 0;
			++it;
		}
		if (size <= 0)
			break;
	}
}

BufferLength SequentialBufferReadGather::availableBytes()
{
	BufferLength length = 0;
	for (auto &b: m_buffers)
		length += b->readableBytes();
	return length;
}

void SequentialBufferReadGather::addBufferNode(const std::shared_ptr<Buffer> &buffer)
{
	m_buffers.emplace_back(buffer);

	using NodesSizeType = std::vector<std::shared_ptr<Buffer>>::size_type;
	NodesSizeType size = m_buffers.size();
	auto *vecNodes = new ::iovec[size];
	for (NodesSizeType i = 0; i < size; ++i)
	{
		auto &bufferNode = m_buffers[i];
		vecNodes[i].iov_base = bufferNode->getRawPointer();
		vecNodes[i].iov_len = bufferNode->readableBytes();
	}

	delete []m_msghdr->msg_iov;
	m_msghdr->msg_iov = vecNodes;
	m_msghdr->msg_iovlen = size - 1;
}

std::shared_ptr<Buffer> SequentialBufferReadGather::getBuffer()
{
	std::shared_ptr<Buffer> buffer;
	if (!m_buffers.empty())
		buffer = m_buffers[0];
	return buffer;
}

/*DatagramReadGather::DatagramReadGather() = default;

DatagramReadGather::~DatagramReadGather() = default;

void DatagramReadGather::adjustByteArray(BufferLength size)
{
}

BufferLength DatagramReadGather::availableBytes()
{
	BufferLength length = 0;
	for (auto &b : m_pendingSend)
		length += b.second.buffer->readableBytes();
	return length;
}

void DatagramReadGather::addBufferNode(const std::shared_ptr<Buffer> &buffer)
{
	addBufferNode(buffer, nullptr);
}

void DatagramReadGather::addBufferNode(const std::shared_ptr<Buffer> &buffer, const Address &destination)
{
	auto address = new ::sockaddr_in;
	toSockAddress(destination, address);

	addBufferNode(buffer, address);
}

void DatagramReadGather::addBufferNode(const std::shared_ptr<Buffer> &buffer, ::sockaddr_in *address)
{
	::msghdr *msg = new ::msghdr;
	std::memset(msg, 0, sizeof(::msghdr));

	auto *vecNodes = new ::iovec[1];
	vecNodes[0].iov_base = buffer->getRawPointer();
	vecNodes[0].iov_len = buffer->readableBytes();
	msg->msg_iov = vecNodes;
	msg->msg_iovlen = 1;
	msg->msg_name = address;
	msg->msg_namelen = address ? sizeof(::sockaddr_in) : 0;

	auto it = m_pendingSend.find(buffer.get());
	if (it != m_pendingSend.end())
	{
		delete it->second.msg;
		it->second.msg = msg;
	}
	else
	{
		m_pendingSend.emplace(buffer.get(), DGData{ buffer, msg });
		m_bufferQueue.push(buffer.get());
	}
}

DatagramWriteGather::DatagramWriteGather(std::shared_ptr<Datagram> data)
{
	m_msghdr->msg_name = datagramDestinationExtractor(data.get());
	m_msghdr->msg_namelen = sizeof(::sockaddr_in);
}

DatagramWriterWithLock::~DatagramWriterWithLock() = default;

void DatagramWriterWithLock::adjustByteArray(ByteArray::LengthType size)
{
	m_writer.adjustByteArray(size);
}

ByteArray::LengthType DatagramWriterWithLock::availableBytes()
{
	return m_writer.availableBytes();
}

::sockaddr_in *extractDestination(Datagram *data)
{
	return data->m_address;
}*/

std::shared_ptr<Buffer> extractBuffer(const ByteArray *byteArray)
{
	return byteArray->m_buffer;
}
}
