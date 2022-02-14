#include "internal/socket/SocketIO.h"
#include "internal/socket/Socket.h"
#include <cstring>
#include "internal/stub/IO.h"
extern "C" {
#include <sys/socket.h>
#include <sys/uio.h>
}

namespace {
bool _write_impl(const netpp::internal::socket::Socket *socket, netpp::internal::socket::ByteArray2IOVec *reader)
{
	std::size_t expectWriteSize = reader->availableBytes();
	::msghdr msg{};
	std::memset(&msg, 0, sizeof(::msghdr));
	msg.msg_iov = reader->iovec();
	msg.msg_iovlen = reader->iovenLength();
	::ssize_t actualSend = netpp::internal::stub::sendmsg(socket->fd(), &msg, MSG_NOSIGNAL);
	if (actualSend != -1)
	{
		auto sendSize = static_cast<std::size_t>(actualSend);
		reader->adjustByteArray(sendSize);
		return (sendSize <= expectWriteSize);
	}
	else
	{
		// error that can not recovery threw as exception
		return false;
	}
}
}

namespace netpp::internal::socket {
ByteArray2IOVec::ByteArray2IOVec()
	: m_vec{nullptr}, m_vecLen{0}
{
}

ByteArray2IOVec::~ByteArray2IOVec()
{
	delete [] m_vec;
}

ByteArrayReaderWithLock::ByteArrayReaderWithLock(std::shared_ptr<ByteArray> buffer)
	: m_buffer{std::move(buffer)}
{
	m_buffer->m_bufferMutex.lock();
	unsigned nodes = m_buffer->m_nodeCount;
	m_vecLen = 0;
	auto *vecNodes = new ::iovec[nodes];
	if (nodes > 0)
	{
		std::shared_ptr<ByteArray::BufferNode> node = m_buffer->_currentReadBufferNode.lock();
		std::shared_ptr<ByteArray::BufferNode> endNode = m_buffer->_currentWriteBufferNode.lock()->next;
		// has node
		// current node is not empty
		while (node != endNode && (node->end - node->start != 0))
		{
			vecNodes[m_vecLen].iov_base = node->buffer + node->start;
			vecNodes[m_vecLen].iov_len = node->end - node->start;
			node = node->next;
			++m_vecLen;
		}
	}
	if (m_vecLen != 0)
		m_vec = vecNodes;
}

ByteArrayReaderWithLock::~ByteArrayReaderWithLock()
{
	m_buffer->m_bufferMutex.unlock();
}

void ByteArrayReaderWithLock::adjustByteArray(ByteArray::LengthType size)
{
	if (size >= m_buffer->m_availableSizeToRead)	// if size is larger than this ByteArray, just read node to end
	{
		m_buffer->m_availableSizeToRead = 0;
		m_buffer->_currentReadBufferNode = m_buffer->_currentWriteBufferNode;
		auto node = m_buffer->_currentReadBufferNode.lock();
		node->start = node->end;
	}
	else
	{
		m_buffer->m_availableSizeToRead -= size;
		ByteArray::LengthType forwardNodes = size / ByteArray::BufferNodeSize;
		std::shared_ptr<ByteArray::BufferNode> node = m_buffer->_currentReadBufferNode.lock();
		for (ByteArray::LengthType i = 0; i < forwardNodes; ++i)
			node = node->next;
		node->start = node->end;
		m_buffer->_currentReadBufferNode = node;
	}
	m_buffer->unlockedMoveBufferHead();
}

ByteArray::LengthType ByteArrayReaderWithLock::availableBytes()
{
	return m_buffer->m_availableSizeToRead;
}

ByteArrayWriterWithLock::ByteArrayWriterWithLock(std::shared_ptr<ByteArray> buffer)
	: m_buffer(std::move(buffer))
{
	m_buffer->m_bufferMutex.lock();
	uint64_t bytes = m_buffer->m_availableSizeToWrite;
	m_vecLen = bytes / ByteArray::BufferNodeSize;
	if (bytes % ByteArray::BufferNodeSize != 0)
		++m_vecLen;
	if (m_vecLen > 0)
	{
		m_vec = new ::iovec[m_vecLen];
		std::shared_ptr<ByteArray::BufferNode> node = m_buffer->_currentWriteBufferNode.lock();
		int i = 0;
		while (node)
		{
			m_vec[i].iov_base = node->buffer + node->end;
			m_vec[i].iov_len = ByteArray::BufferNodeSize - node->end;
			node = node->next;
			++i;
		}
	}
}

ByteArrayWriterWithLock::~ByteArrayWriterWithLock()
{
	m_buffer->m_bufferMutex.unlock();
}

void ByteArrayWriterWithLock::adjustByteArray(ByteArray::LengthType size)
{
	// if need to alloc more
	if (m_buffer->m_availableSizeToWrite <= size)
		m_buffer->unlockedAllocIfNotEnough(size);
	// move write node
	std::shared_ptr<ByteArray::BufferNode> node = m_buffer->_currentWriteBufferNode.lock();
	m_buffer->m_availableSizeToWrite -= size;
	m_buffer->m_availableSizeToRead += size;
	while (size > 0)
	{
		if (size >= (ByteArray::BufferNodeSize - node->end))
		{
			size -= (ByteArray::BufferNodeSize - node->end);
			node->end = ByteArray::BufferNodeSize;
			m_buffer->_currentWriteBufferNode = node;
			node = node->next;
		}
		else
		{
			node->end += size;
			size = 0;
		}
	}
}

ByteArray::LengthType ByteArrayWriterWithLock::availableBytes()
{
	return m_buffer->m_availableSizeToWrite;
}

SequentialByteArrayReaderWithLock::SequentialByteArrayReaderWithLock(std::vector<std::shared_ptr<ByteArray>> &&buffers)
	: m_buffers(std::move(buffers))
{
	unsigned nodeCount = 0;
	for (auto &b : m_buffers)
	{
		b->m_bufferMutex.lock();
		nodeCount += b->m_nodeCount;
	}

	m_vecLen = 0;
	auto *vecNodes = new ::iovec[nodeCount];
	for (auto &b : m_buffers)
	{
		std::shared_ptr<ByteArray::BufferNode> node = b->_currentReadBufferNode.lock();
		std::shared_ptr<ByteArray::BufferNode> endNode = b->_currentWriteBufferNode.lock()->next;
		// has node
		// current node is not empty
		while (node != endNode && (node->end - node->start != 0))
		{
			vecNodes[m_vecLen].iov_base = node->buffer + node->start;
			vecNodes[m_vecLen].iov_len = node->end - node->start;
			node = node->next;
			++m_vecLen;
		}
	}
	if (m_vecLen != 0)
		m_vec = vecNodes;
}

SequentialByteArrayReaderWithLock::~SequentialByteArrayReaderWithLock() noexcept
{
	for (auto &b : m_buffers)
		b->m_bufferMutex.unlock();
}

void SequentialByteArrayReaderWithLock::adjustByteArray(ByteArray::LengthType size)
{
	for (auto &b : m_buffers)
	{
		if (size >= b->m_availableSizeToRead)	// if size is larger than this ByteArray, just read node to end
		{
			size -= b->m_availableSizeToRead;
			b->m_availableSizeToRead = 0;
			b->_currentReadBufferNode = b->_currentWriteBufferNode;
			auto node = b->_currentReadBufferNode.lock();
			node->start = node->end;
		}
		else
		{
			b->m_availableSizeToRead -= size;
			size = 0;
			ByteArray::LengthType forwardNodes = size / ByteArray::BufferNodeSize;
			std::shared_ptr<ByteArray::BufferNode> node = b->_currentReadBufferNode.lock();
			for (ByteArray::LengthType i = 0; i < forwardNodes; ++i)
				node = node->next;
			node->start = node->end;
			b->_currentReadBufferNode = node;
		}
		b->unlockedMoveBufferHead();
		if (size <= 0)
			break;
	}
}

ByteArray::LengthType SequentialByteArrayReaderWithLock::availableBytes()
{
	ByteArray::LengthType length = 0;
	for (auto &b : m_buffers)
		length += b->m_availableSizeToRead;
	return length;
}

// SocketIO
void SocketIO::read(const Socket *socket, std::shared_ptr<ByteArray> buffer)
{
	ByteArrayWriterWithLock writer(std::move(buffer));
	::msghdr msg{};
	std::memset(&msg, 0, sizeof(::msghdr));
	msg.msg_iov = writer.iovec();
	msg.msg_iovlen = writer.iovenLength();
	// TODO: use ioctl(fd, FIONREAD, &n) to get pending read data on socket
	::ssize_t num = stub::recvmsg(socket->fd(), &msg, 0);
	if (num != -1)
		writer.adjustByteArray(static_cast<std::size_t>(num));
}

bool SocketIO::write(const Socket *socket, std::shared_ptr<ByteArray> buffer)
{
	ByteArrayReaderWithLock reader(std::move(buffer));
	return ::_write_impl(socket, &reader);
}

bool SocketIO::write(const Socket *socket, std::vector<std::shared_ptr<ByteArray>> &&buffers)
{
	SequentialByteArrayReaderWithLock reader(std::move(buffers));
	return ::_write_impl(socket, &reader);
}
}
