//
// Created by gaojian on 2022/3/25.
//

#include "buffer/ChannelBufferConversion.h"
#include "buffer/TcpChannel.h"
#include "buffer/BufferIOConversion.h"

namespace netpp {
std::unique_ptr<BufferIOConversion> TcpChannelConversion::readBufferConvert(Channel *channel)
{
	auto tcpChannel = dynamic_cast<TcpChannel *>(channel);
	return std::make_unique<ByteArrayReaderWithLock>(tcpChannel->m_sendBuffer);
}

std::unique_ptr<BufferIOConversion> TcpChannelConversion::writeBufferConvert(Channel *channel)
{
	auto tcpChannel = dynamic_cast<TcpChannel *>(channel);
	return std::make_unique<ByteArrayWriterWithLock>(tcpChannel->m_receiveArray);
}

std::unique_ptr<BufferIOConversion> HttpChannelConversion::readBufferConvert(Channel *channel)
{
	auto httpChannel = dynamic_cast<HttpChannel *>(channel);
	return std::make_unique<ByteArrayReaderWithLock>(httpChannel->m_receiveArray);
}

std::unique_ptr<BufferIOConversion> HttpChannelConversion::writeBufferConvert(Channel *channel)
{
	auto httpChannel = dynamic_cast<HttpChannel *>(channel);
	return std::make_unique<SequentialByteArrayReaderWithLock>(
			std::forward<std::initializer_list<std::shared_ptr<ByteArray>>>(
			{httpChannel->m_sendHeaderArray, httpChannel->m_sendBodyArray}
	)
	);
}
}
