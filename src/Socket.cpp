//
// Created by gaojian on 2020/7/5.
//

#include "Socket.h"
#include "Log.h"
#include "ByteArray.h"
#include <cstring>
extern "C" {
#include <sys/socket.h>
#include <sys/uio.h>
#include <unistd.h>
}

using std::make_unique;

namespace netpp {
Socket::Socket()
{
	if ((m_socketFd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP)) == -1)
		SPDLOG_LOGGER_CRITICAL(logger, "Failed to create Socket: {}", std::string(strerror(errno)));
	SPDLOG_LOGGER_DEBUG(logger, "Socket fd: {}", m_socketFd);
}

Socket::Socket(const Address &addr)
	: Socket()
{
	m_addr = addr;
}

Socket::Socket(int fd, const Address &addr)
	: m_socketFd{fd}, m_addr{addr}
{
	if (m_socketFd == -1)
		SPDLOG_LOGGER_ERROR(logger, "Socket fd is -1");
}

Socket::~Socket()
{
	SPDLOG_LOGGER_TRACE(logger, "socket {} destructed", m_socketFd);
	if (m_socketFd != -1)
		::close(m_socketFd);
}

void Socket::listen()
{
	if (m_socketFd == -1)
	{
		SPDLOG_LOGGER_CRITICAL(logger, "Invalid fd -1");
		return;
	}
	const sockaddr_in *inetAddr = m_addr.sockAddrIn();
	if (::bind(m_socketFd, reinterpret_cast<const sockaddr *>(inetAddr), sizeof(*inetAddr)) == -1)
	{
		std::string errnoStr = strerror(errno);
		SPDLOG_LOGGER_CRITICAL(logger, "Failed to bind address: {}", errnoStr);
		throw std::runtime_error("Failed to bind address " + errnoStr);
	}
	if (::listen(m_socketFd, 10) == -1)
	{
		std::string errnoStr = strerror(errno);
		SPDLOG_LOGGER_CRITICAL(logger, "Failed to listen: {}", errnoStr);
		throw std::runtime_error("Failed to listen: " + errnoStr);
	}
	SPDLOG_LOGGER_TRACE(logger, "Start listen");
}

std::unique_ptr<Socket> Socket::accept() const
{
	if (m_socketFd == -1)
	{
		SPDLOG_LOGGER_ERROR(logger, "Invalid fd -1");
		return nullptr;
	}
	int newSocket;
	sockaddr_in inetAddr{0};
	socklen_t addrSize = sizeof(inetAddr);
	if ((newSocket = ::accept(m_socketFd, reinterpret_cast<sockaddr *>(&inetAddr), &addrSize)) == -1)
	{
		SPDLOG_LOGGER_ERROR(logger, "Failed to accept: {}", std::string(strerror(errno)));
		return nullptr;
	}
	SPDLOG_LOGGER_TRACE(logger, "Accepted new client with fd {}", newSocket);
	return make_unique<Socket>(newSocket, Address(inetAddr));
}

void Socket::connect()
{
	if (m_socketFd == -1)
	{
		SPDLOG_LOGGER_CRITICAL(logger, "Invalid fd -1");
		return;
	}
	const sockaddr_in *inetAddr = m_addr.sockAddrIn();
	::connect(m_socketFd, reinterpret_cast<const sockaddr *>(inetAddr), sizeof(*inetAddr));
}

void Socket::shutdownWrite()
{
	if (m_socketFd != -1)
	{
		::shutdown(m_socketFd, SHUT_WR);
		SPDLOG_LOGGER_TRACE(logger, "Shut down write for socket {}", m_socketFd);
	}
	else
		SPDLOG_LOGGER_CRITICAL(logger, "Invalid fd -1");
}

int Socket::getError() const
{
	int optval;
	socklen_t optlen = static_cast<socklen_t>(sizeof optval);
	if (::getsockopt(m_socketFd, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0)
		return errno;
	else
		return optval;
}

// void Socket::close() const
// {
// 	SPDLOG_LOGGER_TRACE(logger, "Closing {}", m_socketFd);
// 	::close(m_socketFd);
// }

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
	vec.adjustByteArray(num);
}

void SocketIO::write(const Socket *socket, std::shared_ptr<ByteArray> buffer)
{
	ByteArrayIOVectorReaderWithLock vec(buffer);
	std::size_t num = ::writev(socket->fd(), vec.vec(), vec.count());
	vec.adjustByteArray(num);
}
}
