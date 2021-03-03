#include "socket/SocketIO.h"
#include "socket/Socket.h"
#include <cstring>
#include "stub/IO.h"
extern "C" {
#include <sys/socket.h>
#include <sys/uio.h>
}

namespace netpp::internal::socket {
ByteArray2Msghdr::ByteArray2Msghdr()
	: msg{nullptr}, _buffer{nullptr}
{
	msg = new ::msghdr;
	std::memset(msg, 0, sizeof(::msghdr));
}

ByteArray2Msghdr::~ByteArray2Msghdr()
{
	if (msg->msg_iovlen != 0)
		delete [] msg->msg_iov;
	delete msg;
}

ByteArrayReaderWithLock::ByteArrayReaderWithLock(std::shared_ptr<ByteArray> buffer)
{
	_buffer = buffer;
	_buffer->m_bufferMutex.lock();
	uint64_t bytes = buffer->m_availableSizeToRead;
	::size_t vecCount = bytes / ByteArray::BufferNode::BufferSize;
	if (bytes % ByteArray::BufferNode::BufferSize != 0)
		++vecCount;
	if (vecCount > 0)
	{
		::iovec *vec = new ::iovec[vecCount];
		std::shared_ptr<ByteArray::BufferNode> node = buffer->_currentReadBufferNode.lock();
		std::shared_ptr<ByteArray::BufferNode> endNode = buffer->_currentWriteBufferNode.lock()->next;
		int i = 0;
		while (node != endNode)
		{
			vec[i].iov_base = node->buffer + node->start;
			if (node->next == endNode)
				vec[i].iov_len = node->end - node->start;
			else
				vec[i].iov_len = ByteArray::BufferNode::BufferSize;
			node = node->next;
			++i;
		}
		msg->msg_iov = vec;
		msg->msg_iovlen = vecCount;
	}
}

ByteArrayReaderWithLock::~ByteArrayReaderWithLock()
{
	_buffer->m_bufferMutex.unlock();
}

void ByteArrayReaderWithLock::adjustByteArray(ByteArray::LengthType size)
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
	_buffer->unlockedMoveBufferHead();
}

ByteArray::LengthType ByteArrayReaderWithLock::availableBytes()
{
	// not use ByteArray::readableBytes(), lock acquired here
	return _buffer->m_availableSizeToRead;
}

ByteArrayWriterWithLock::ByteArrayWriterWithLock(std::shared_ptr<ByteArray> buffer)
{
	_buffer = buffer;
	_buffer->m_bufferMutex.lock();
	uint64_t bytes = buffer->m_availableSizeToWrite;
	::size_t vecCount = bytes / ByteArray::BufferNode::BufferSize;
	if (bytes % ByteArray::BufferNode::BufferSize != 0)
		++vecCount;
	if (vecCount > 0)
	{
		::iovec *vec = new ::iovec[vecCount];
		std::shared_ptr<ByteArray::BufferNode> node = buffer->_currentWriteBufferNode.lock();
		int i = 0;
		while (node)
		{
			vec[i].iov_base = node->buffer + node->end;
			vec[i].iov_len = ByteArray::BufferNode::BufferSize - node->end;
			node = node->next;
			++i;
		}
		msg->msg_iov = vec;
		msg->msg_iovlen = vecCount;
	}
}

ByteArrayWriterWithLock::~ByteArrayWriterWithLock()
{
	_buffer->m_bufferMutex.unlock();
}

void ByteArrayWriterWithLock::adjustByteArray(ByteArray::LengthType size)
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
		if (size >= (ByteArray::BufferNode::BufferSize - node->end))
		{
			size -= (ByteArray::BufferNode::BufferSize - node->end);
			node->end = ByteArray::BufferNode::BufferSize;
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

ByteArray::LengthType ByteArrayWriterWithLock::availableBytes()
{
	// not use ByteArray::writeableBytes(), lock acquired here
	return _buffer->m_availableSizeToWrite;
}

// SocketIO
void SocketIO::read(const Socket *socket, std::shared_ptr<ByteArray> buffer)
{
	ByteArrayWriterWithLock writer(buffer);
	::msghdr *msg = writer.msghdr();
	// TODO: use ioctl(fd, FIONREAD, &n) to get pending read data on socket
	::ssize_t num = stub::recvmsg(socket->fd(), msg, 0);
	if (num != -1)
		writer.adjustByteArray(static_cast<std::size_t>(num));
}

bool SocketIO::write(const Socket *socket, std::shared_ptr<ByteArray> buffer)
{
	ByteArrayReaderWithLock vec(buffer);
	std::size_t expectSize = vec.availableBytes();
	::msghdr *msg = vec.msghdr();
	::ssize_t actualSend = stub::sendmsg(socket->fd(), msg, MSG_NOSIGNAL);
	if (actualSend != -1)
	{
		std::size_t sendSize = static_cast<std::size_t>(actualSend);
		vec.adjustByteArray(sendSize);
		return (sendSize <= expectSize);
	}
	else
	{
		// error that can not recovery throwed as exception
		return false;
	}
}
}
