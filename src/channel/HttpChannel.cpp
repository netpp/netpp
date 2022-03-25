//
// Created by gaojian on 2022/3/24.
//

#include "channel/HttpChannel.h"
#include "ByteArray.h"
#include "internal/buffer/ChannelBufferConversion.h"

namespace netpp {
HttpChannel::HttpChannel(std::weak_ptr<internal::handlers::TcpConnection> connection)
		: Channel(std::move(connection)),
		  m_receiveArray{std::make_shared<ByteArray>()},
		  m_sendHeaderArray{std::make_shared<ByteArray>()}, m_sendBodyArray{std::make_shared<ByteArray>()}
{}

std::unique_ptr<internal::buffer::ChannelBufferConversion> HttpChannel::createBufferConvertor()
{
	return std::make_unique<internal::buffer::HttpChannelConversion>();
}
}
