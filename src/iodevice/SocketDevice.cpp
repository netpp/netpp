//
// Created by 17271 on 2022/6/5.
//

#include "iodevice/SocketDevice.h"
#include "buffer/BufferIOConversion.h"
#include <cstring>
extern "C" {
#include <sys/socket.h>
#include <sys/uio.h>
}

namespace netpp {
void SocketDevice::read(std::unique_ptr<BufferIOConversion> &&bufferConverter)
{
	::msghdr msg{};
	std::memset(&msg, 0, sizeof(::msghdr));
	msg.msg_iov = bufferConverter->iovec();
	msg.msg_iovlen = bufferConverter->iovenLength();
	// TODO: use ioctl(fd, FIONREAD, &n) to get pending read data on socket
	std::size_t num = realRecv(&msg);
	bufferConverter->adjustByteArray(static_cast<std::size_t>(num));
}

bool SocketDevice::write(std::unique_ptr<BufferIOConversion> &&bufferConverter)
{
	std::size_t expectWriteSize = bufferConverter->availableBytes();
	::msghdr msg{};
	std::memset(&msg, 0, sizeof(::msghdr));
	msg.msg_iov = bufferConverter->iovec();
	msg.msg_iovlen = bufferConverter->iovenLength();
	std::size_t actualSend = realSend(&msg);

	auto sendSize = static_cast<std::size_t>(actualSend);
	bufferConverter->adjustByteArray(sendSize);
	return (sendSize <= expectWriteSize);
}
}