//
// Created by 17271 on 2022/6/5.
//

#include "iodevice/SocketDevice.h"
#include "buffer/BufferGather.h"
#include <cstring>
extern "C" {
#include <sys/socket.h>
#include <sys/uio.h>
}

namespace netpp {
void SocketDevice::read(std::shared_ptr<BufferGather> buffer)
{
	// TODO: use ioctl(fd, FIONREAD, &n) to get pending read data on socket
	std::size_t num = realRecv(buffer->msghdr());
	buffer->adjustByteArray(static_cast<std::size_t>(num));
}

std::size_t SocketDevice::write(std::shared_ptr<BufferGather> buffer)
{
//	std::size_t expectWriteSize = bufferConverter->availableBytes();
	std::size_t actualSend = realSend(buffer->msghdr());

	auto sendSize = static_cast<std::size_t>(actualSend);
	buffer->adjustByteArray(sendSize);

	return actualSend;
}
}
