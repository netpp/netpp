//
// Created by gaojian on 2022/3/25.
//

#include "internal/buffer/ChannelBufferConversion.h"
#include "channel/TcpChannel.h"
#include "internal/socket/SocketIO.h"

namespace netpp::internal::buffer {
std::unique_ptr<internal::socket::ByteArray2IOVec> TcpChannelConversion::readBufferConvert(Channel *channel)
{
	auto tcpChannel = dynamic_cast<TcpChannel *>(channel);
	return std::make_unique<internal::socket::ByteArrayReaderWithLock>(tcpChannel->m_sendBuffer);
}

std::unique_ptr<internal::socket::ByteArray2IOVec> TcpChannelConversion::writeBufferConvert(Channel *channel)
{
	auto tcpChannel = dynamic_cast<TcpChannel *>(channel);
	return std::make_unique<internal::socket::ByteArrayWriterWithLock>(tcpChannel->m_receiveArray);
}
}
