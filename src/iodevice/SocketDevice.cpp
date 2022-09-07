//
// Created by 17271 on 2022/6/5.
//

#include "iodevice/SocketDevice.h"
#include "buffer/BufferIOConversion.h"
#include <cstring>
#include "buffer/TransferBuffer.h"
extern "C" {
#include <sys/socket.h>
#include <sys/uio.h>
}

namespace netpp {
void SocketDevice::read(std::shared_ptr<TransferBuffer> buffer)
{
	auto bufferConverter = buffer->receiveBufferForIO();
	// TODO: use ioctl(fd, FIONREAD, &n) to get pending read data on socket
	std::size_t num = realRecv(bufferConverter->msghdr());
	bufferConverter->adjustByteArray(static_cast<std::size_t>(num));
}

std::size_t SocketDevice::write(std::shared_ptr<TransferBuffer> buffer)
{
	auto bufferConverter = buffer->sendBufferForIO();
//	std::size_t expectWriteSize = bufferConverter->availableBytes();
	std::size_t actualSend = realSend(bufferConverter->msghdr());

	auto sendSize = static_cast<std::size_t>(actualSend);
	bufferConverter->adjustByteArray(sendSize);

	return actualSend;
}
}
