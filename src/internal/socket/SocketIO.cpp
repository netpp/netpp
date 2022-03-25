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
	: m_vec{nullptr}, m_vecLen{0}
{
}

ByteArray2IOVec::~ByteArray2IOVec()
{
	delete [] m_vec;
}

ByteArrayReaderWithLock::ByteArrayReaderWithLock(std::shared_ptr<ByteArray> buffer)
	: m_buffer{std::move(buffer)}, m_lck{m_buffer->m_bufferMutex}
{
	ByteArray::CowBuffer::NodeContainerIndexer endOfRead = m_buffer->endOfReadNode();
	m_vecLen = 0;
	auto *vecNodes = new ::iovec[endOfRead - m_buffer->m_readNode];

	for (auto it = m_buffer->m_nodes->cowBegin(m_buffer->m_readNode); (it <=> m_buffer->m_nodes->cowEnd(endOfRead)) < 0; ++it)
	{
		if (it->end - it->start != 0)		// this node is empty
		{
			vecNodes[m_vecLen].iov_base = it->buffer + it->start;
			vecNodes[m_vecLen].iov_len = it->end - it->start;
			++m_vecLen;
		}
	}
	if (m_vecLen != 0)
		m_vec = vecNodes;
}

ByteArrayReaderWithLock::~ByteArrayReaderWithLock() = default;

void ByteArrayReaderWithLock::adjustByteArray(ByteArray::LengthType size)
{
	if (size >= m_buffer->m_availableSizeToRead)	// if size is larger than this ByteArray, just read node to end
	{
		m_buffer->m_readNode = m_buffer->m_writeNode;
		m_buffer->m_availableSizeToRead = 0;
		auto it = m_buffer->m_nodes->cowBegin(m_buffer->m_readNode);
		it->start = it->end;
	}
	else
	{
		m_buffer->m_availableSizeToRead -= size;
		ByteArray::CowBuffer::NodeContainerIndexer moveForward = size / ByteArray::BufferNodeSize;
		ByteArray::CowBuffer::NodeContainerIndexer left = size % ByteArray::BufferNodeSize;
		moveForward += (moveForward != 0 && left != 0);
		m_buffer->m_readNode += moveForward;
		auto it = m_buffer->m_nodes->cowBegin(m_buffer->m_readNode);
		it->start += left;
	}
	m_buffer->unlockedMoveBufferHead();
}

ByteArray::LengthType ByteArrayReaderWithLock::availableBytes()
{
	return m_buffer->m_availableSizeToRead;
}

ByteArrayWriterWithLock::ByteArrayWriterWithLock(std::shared_ptr<ByteArray> buffer)
	: m_buffer(std::move(buffer)), m_lck{m_buffer->m_bufferMutex}
{
	uint64_t bytes = m_buffer->m_availableSizeToWrite;
	m_vecLen = bytes / ByteArray::BufferNodeSize + (bytes % ByteArray::BufferNodeSize != 0);
	if (m_vecLen > 0)
	{
		m_vec = new ::iovec[m_vecLen];
		int i = 0;
		for (auto it = m_buffer->m_nodes->cowBegin(m_buffer->m_writeNode); (it <=> m_buffer->m_nodes->cowEnd(m_buffer->m_nodes->size())) < 0; ++it)
		{
			m_vec[i].iov_base = it->buffer + it->end;
			m_vec[i].iov_len = ByteArray::BufferNodeSize - it->end;
			++i;
		}
	}
}

ByteArrayWriterWithLock::~ByteArrayWriterWithLock() = default;

void ByteArrayWriterWithLock::adjustByteArray(ByteArray::LengthType size)
{
	// if need to alloc more
	if (m_buffer->m_availableSizeToWrite <= size)
		m_buffer->unlockedAllocIfNotEnough(size);
	m_buffer->m_availableSizeToWrite -= size;
	m_buffer->m_availableSizeToRead += size;

	for (auto it = m_buffer->m_nodes->cowBegin(m_buffer->m_writeNode); (it <=> m_buffer->m_nodes->cowEnd(m_buffer->m_nodes->size())) < 0; ++it)
	{
		it->end = (it->end + size >= ByteArray::BufferNodeSize) ? ByteArray::BufferNodeSize : (it->end + size);
		size -= (ByteArray::BufferNodeSize - it->end);
		if (m_buffer->m_writeNode + 1 < m_buffer->m_nodes->size())
			++m_buffer->m_writeNode;
	}
}

ByteArray::LengthType ByteArrayWriterWithLock::availableBytes()
{
	return m_buffer->m_availableSizeToWrite;
}

SequentialByteArrayReaderWithLock::SequentialByteArrayReaderWithLock(std::initializer_list<std::shared_ptr<ByteArray>> buffers)
	: SequentialByteArrayReaderWithLock(std::vector(buffers))
{}

SequentialByteArrayReaderWithLock::SequentialByteArrayReaderWithLock(std::vector<std::shared_ptr<ByteArray>> &&buffers)
	: m_buffers(buffers)
{
	ByteArray::CowBuffer::NodeContainerIndexer nodeCount = 0;
	for (auto &b : m_buffers)
	{
		m_lck.emplace_back(std::make_unique<std::lock_guard<ByteArrayMutex>>(b->m_bufferMutex));
		ByteArray::CowBuffer::NodeContainerIndexer size = b->m_nodes->size();
		if (size > 0)
			nodeCount += size;
	}

	if (nodeCount != 0)
	{
		m_vecLen = 0;
		auto *vecNodes = new ::iovec[nodeCount];
		for (auto &b: m_buffers)
		{
			ByteArray::CowBuffer::NodeContainerIndexer endOfRead = b->endOfReadNode();
			for (auto it = b->m_nodes->cowBegin(b->m_readNode); (it <=> b->m_nodes->cowEnd(endOfRead)) < 0; ++it)
			{
				if (it->end - it->start != 0)        // this node is empty
				{
					vecNodes[m_vecLen].iov_base = it->buffer + it->start;
					vecNodes[m_vecLen].iov_len = it->end - it->start;
					++m_vecLen;
				}
			}
		}
	if (m_vecLen != 0)
		m_vec = vecNodes;
	}
}

SequentialByteArrayReaderWithLock::~SequentialByteArrayReaderWithLock() = default;

void SequentialByteArrayReaderWithLock::adjustByteArray(ByteArray::LengthType size)
{
	for (auto &b : m_buffers)
	{
		if (size >= b->m_availableSizeToRead)	// if size is larger than this ByteArray, just read node to end
		{
			size -= b->m_availableSizeToRead;
			b->m_readNode = b->m_writeNode;
			b->m_availableSizeToRead = 0;
			auto it = b->m_nodes->cowBegin(b->m_readNode);
			it->start = it->end;
		}
		else
		{
			b->m_availableSizeToRead -= size;
			ByteArray::CowBuffer::NodeContainerIndexer moveForward = size / ByteArray::BufferNodeSize;
			ByteArray::CowBuffer::NodeContainerIndexer left = size % ByteArray::BufferNodeSize;
			moveForward += (moveForward != 0 && left != 0);
			size = 0;
			b->m_readNode += moveForward;
			auto it = b->m_nodes->cowBegin(b->m_readNode);
			it->start += left;
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
void SocketIO::read(const Socket *socket, std::unique_ptr<ByteArray2IOVec> &&writer)
{
	::msghdr msg{};
	std::memset(&msg, 0, sizeof(::msghdr));
	msg.msg_iov = writer->iovec();
	msg.msg_iovlen = writer->iovenLength();
	// TODO: use ioctl(fd, FIONREAD, &n) to get pending read data on socket
	::ssize_t num = stub::recvmsg(socket->fd(), &msg, 0);
	if (num != -1)
		writer->adjustByteArray(static_cast<std::size_t>(num));
}

bool SocketIO::write(const Socket *socket, std::unique_ptr<ByteArray2IOVec> &&reader)
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
