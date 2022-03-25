//
// Created by kojiko on 2022/3/25.
//

#include "internal/buffer/ChannelBufferConversion.h"
#include "channel/TcpChannel.h"
#include "channel/HttpChannel.h"
#include "internal/socket/SocketIO.h"

namespace netpp::internal::buffer {
std::unique_ptr<internal::socket::ByteArray2IOVec> TcpChannelConversion::readBufferConvert(Channel *channel)
{
auto tcpChannel = dynamic_cast<TcpChannel *>(channel);
return std::make_unique<internal::socket::ByteArrayReaderWithLock>(tcpChannel->m_readArray);
}

std::unique_ptr<internal::socket::ByteArray2IOVec> TcpChannelConversion::writeBufferConvert(Channel *channel)
{
auto tcpChannel = dynamic_cast<TcpChannel *>(channel);
return std::make_unique<internal::socket::ByteArrayReaderWithLock>(tcpChannel->m_writeArray);
}

std::unique_ptr<internal::socket::ByteArray2IOVec> HttpChannelConversion::readBufferConvert(Channel *channel)
{
auto tcpChannel = dynamic_cast<HttpChannel *>(channel);
return std::make_unique<internal::socket::ByteArrayReaderWithLock>(tcpChannel->m_receiveArray);
}

std::unique_ptr<internal::socket::ByteArray2IOVec> HttpChannelConversion::writeBufferConvert(Channel *channel)
{
auto tcpChannel = dynamic_cast<HttpChannel *>(channel);
return std::make_unique<internal::socket::SequentialByteArrayReaderWithLock>(
		std::forward<std::initializer_list<std::shared_ptr<ByteArray>>>({tcpChannel->m_sendHeaderArray, tcpChannel->m_sendBodyArray})
);
}
}
