#include "socket/SocketIO.h"
#include "socket/Socket.h"
#include "ByteArray.h"
extern "C" {
#include <sys/socket.h>
#include <sys/uio.h>
#include <unistd.h>
}

namespace netpp::socket {
ByteArray2IOVector::ByteArray2IOVector()
	: m_vec{nullptr}, m_count{0}, _buffer{nullptr}
{}

ByteArray2IOVector::~ByteArray2IOVector() { delete [] m_vec; }

ByteArrayIOVectorReaderWithLock::ByteArrayIOVectorReaderWithLock(std::shared_ptr<ByteArray> buffer)
{
	_buffer = buffer;
	_buffer->m_bufferMutex.lock();
	uint64_t bytes = buffer->m_availableSizeToRead;
	m_count = bytes / ByteArray::BufferNode::BufferSize;
	if (bytes % ByteArray::BufferNode::BufferSize != 0)
		++m_count;
	if (m_count > 0)
	{
		m_vec = new iovec[m_count];
		std::shared_ptr<ByteArray::BufferNode> node = buffer->_currentReadBufferNode.lock();
		std::shared_ptr<ByteArray::BufferNode> endNode = buffer->_currentWriteBufferNode.lock()->next;
		int i = 0;
		while (node != endNode)
		{
			m_vec[i].iov_base = node->buffer + node->start;
			if (node->next == endNode)
				m_vec[i].iov_len = node->end - node->start;
			else
				m_vec[i].iov_len = ByteArray::BufferNode::BufferSize;
			node = node->next;
			++i;
		}
	}
}

ByteArrayIOVectorReaderWithLock::~ByteArrayIOVectorReaderWithLock()
{
	_buffer->m_bufferMutex.unlock();
}

void ByteArrayIOVectorReaderWithLock::adjustByteArray(std::size_t size)
{
	// move read node
	std::shared_ptr<ByteArray::BufferNode> node = _buffer->_currentReadBufferNode.lock();
	_buffer->m_availableSizeToRead -= size;
	while (size > 0)
	{
		if (size > ByteArray::BufferNode::BufferSize)
		{
			node->start = ByteArray::BufferNode::BufferSize;
			size -= ByteArray::BufferNode::BufferSize;
		}
		else
		{
			node->start += size;
			size = 0;
		}
		_buffer->_currentReadBufferNode = node;
		node = node->next;
	}
	_buffer->moveBufferHead();
}

ByteArrayIOVectorWriterWithLock::ByteArrayIOVectorWriterWithLock(std::shared_ptr<ByteArray> buffer)
{
	_buffer = buffer;
	_buffer->m_bufferMutex.lock();
	uint64_t bytes = buffer->m_availableSizeToWrite;
	m_count = bytes / ByteArray::BufferNode::BufferSize;
	if (bytes % ByteArray::BufferNode::BufferSize != 0)
		++m_count;
	if (m_count > 0)
	{
		m_vec = new iovec[m_count];
		std::shared_ptr<ByteArray::BufferNode> node = buffer->_currentWriteBufferNode.lock();
		int i = 0;
		while (node)
		{
			m_vec[i].iov_base = node->buffer + node->end;
			if (!node->next)
				m_vec[i].iov_len = ByteArray::BufferNode::BufferSize - node->end;
			else
				m_vec[i].iov_len = ByteArray::BufferNode::BufferSize;
			node = node->next;
			++i;
		}
	}
}

ByteArrayIOVectorWriterWithLock::~ByteArrayIOVectorWriterWithLock()
{
	_buffer->m_bufferMutex.unlock();
}

void ByteArrayIOVectorWriterWithLock::adjustByteArray(std::size_t size)
{
	// move write node
	std::size_t allocSize = size;
	std::shared_ptr<ByteArray::BufferNode> node = _buffer->_currentWriteBufferNode.lock();
	_buffer->m_availableSizeToWrite -= size;
	_buffer->m_availableSizeToRead += size;
	while (size > 0)
	{
		if (size > ByteArray::BufferNode::BufferSize)
		{
			node->end = ByteArray::BufferNode::BufferSize;
			size -= ByteArray::BufferNode::BufferSize;
		}
		else
		{
			node->end += size;
			size = 0;
		}
		_buffer->_currentWriteBufferNode = node;
		node = node->next;
	}
	// if need to alloc more
	if (_buffer->m_availableSizeToWrite == 0)
		_buffer->allocIfNotEnough(allocSize);
}

// SocketIO
void SocketIO::read(const Socket *socket, std::shared_ptr<ByteArray> buffer)
{
	ByteArrayIOVectorWriterWithLock vec(buffer);
	std::size_t num = ::readv(socket->fd(), vec.vec(), vec.count());
	if (num == -1)
	{
		// TODO: handle readv error
		switch (errno)
		{
			case EINVAL:
			case EOPNOTSUPP:
			default:
				break;
		}
	}
	vec.adjustByteArray(num);
}

bool SocketIO::write(const Socket *socket, std::shared_ptr<ByteArray> buffer)
{
	ByteArrayIOVectorReaderWithLock vec(buffer);
	std::size_t expectSize = buffer->readableBytes();
	std::size_t num = ::writev(socket->fd(), vec.vec(), vec.count());
	if (num == -1)
	{
		// TODO: handle writev error
		switch (errno)
		{
			case EINVAL:
			case EOPNOTSUPP:
			default:
				break;
		}
	}
	vec.adjustByteArray(num);
	return (expectSize > num);
}
}
