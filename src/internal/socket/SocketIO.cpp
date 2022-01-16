#include "internal/socket/SocketIO.h"
#include "internal/socket/Socket.h"
#include <cstring>
#include "internal/stub/IO.h"
extern "C" {
#include <sys/socket.h>
#include <sys/uio.h>
}

namespace netpp::internal::socket {
ByteArray2IOVec::ByteArray2IOVec()
	: m_vec{nullptr}, m_vecLen{0}, _buffer{nullptr}
{
}

ByteArray2IOVec::~ByteArray2IOVec()
{
	delete [] m_vec;
}

ByteArrayIOVecReaderWithLock::ByteArrayIOVecReaderWithLock(std::shared_ptr<ByteArray> buffer)
{
	_buffer = std::move(buffer);
	_buffer->m_bufferMutex.lock();
	unsigned nodes = _buffer->m_nodeCount;
	m_vecLen = 0;
	auto *vecNodes = new ::iovec[nodes];
	if (nodes > 0)
	{
		std::shared_ptr<ByteArray::BufferNode> node = _buffer->_currentReadBufferNode.lock();
		std::shared_ptr<ByteArray::BufferNode> endNode = _buffer->_currentWriteBufferNode.lock()->next;
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

ByteArrayIOVecReaderWithLock::~ByteArrayIOVecReaderWithLock()
{
	_buffer->m_bufferMutex.unlock();
}

void ByteArrayIOVecReaderWithLock::adjustByteArray(ByteArray::LengthType size)
{
	// move read node
	std::shared_ptr<ByteArray::BufferNode> node = _buffer->_currentReadBufferNode.lock();
	_buffer->m_availableSizeToRead -= size;
	while (size > 0)
	{
		if (size >= (ByteArray::BufferNodeSize - node->start))
		{
			size -= (ByteArray::BufferNodeSize - node->start);
			node->start = ByteArray::BufferNodeSize;
			node->end = ByteArray::BufferNodeSize;
		}
		else
		{
			node->start += size;
			size = 0;
		}
		// no matter what, read node would point to current node
		_buffer->_currentReadBufferNode = node;
		node = node->next;
	}
	_buffer->unlockedMoveBufferHead();
}

ByteArray::LengthType ByteArrayIOVecReaderWithLock::availableBytes()
{
	// not use ByteArray::readableBytes(), lock acquired here
	return _buffer->m_availableSizeToRead;
}

ByteArrayIOVecWriterWithLock::ByteArrayIOVecWriterWithLock(std::shared_ptr<ByteArray> buffer)
{
	_buffer = std::move(buffer);
	_buffer->m_bufferMutex.lock();
	uint64_t bytes = _buffer->m_availableSizeToWrite;
	m_vecLen = bytes / ByteArray::BufferNodeSize;
	if (bytes % ByteArray::BufferNodeSize != 0)
		++m_vecLen;
	if (m_vecLen > 0)
	{
		m_vec = new ::iovec[m_vecLen];
		std::shared_ptr<ByteArray::BufferNode> node = _buffer->_currentWriteBufferNode.lock();
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

ByteArrayIOVecWriterWithLock::~ByteArrayIOVecWriterWithLock()
{
	_buffer->m_bufferMutex.unlock();
}

void ByteArrayIOVecWriterWithLock::adjustByteArray(ByteArray::LengthType size)
{
	// if need to alloc more
	if (_buffer->m_availableSizeToWrite <= size)
		_buffer->unlockedAllocIfNotEnough(size);
	// move write node
	std::shared_ptr<ByteArray::BufferNode> node = _buffer->_currentWriteBufferNode.lock();
	_buffer->m_availableSizeToWrite -= size;
	_buffer->m_availableSizeToRead += size;
	while (size > 0)
	{
		if (size >= (ByteArray::BufferNodeSize - node->end))
		{
			size -= (ByteArray::BufferNodeSize - node->end);
			node->end = ByteArray::BufferNodeSize;
			_buffer->_currentWriteBufferNode = node;
			node = node->next;
		}
		else
		{
			node->end += size;
			size = 0;
		}
	}
}

ByteArray::LengthType ByteArrayIOVecWriterWithLock::availableBytes()
{
	// not use ByteArray::writeableBytes(), lock acquired here
	return _buffer->m_availableSizeToWrite;
}

// SocketIO
void SocketIO::read(const Socket *socket, std::shared_ptr<ByteArray> buffer)
{
	ByteArrayIOVecWriterWithLock writer(std::move(buffer));
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
	ByteArrayIOVecReaderWithLock reader(std::move(buffer));
	std::size_t expectWriteSize = reader.availableBytes();
	::msghdr msg{};
	std::memset(&msg, 0, sizeof(::msghdr));
	msg.msg_iov = reader.iovec();
	msg.msg_iovlen = reader.iovenLength();
	::ssize_t actualSend = stub::sendmsg(socket->fd(), &msg, MSG_NOSIGNAL);
	if (actualSend != -1)
	{
		auto sendSize = static_cast<std::size_t>(actualSend);
		reader.adjustByteArray(sendSize);
		return (sendSize <= expectWriteSize);
	}
	else
	{
		// error that can not recovery threw as exception
		return false;
	}
}
}
