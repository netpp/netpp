//
// Created by gaojian on 2022/3/27.
//

#include "internal/http/channel/HttpChannelConversion.h"
#include "internal/http/channel/HttpChannel.h"
#include "internal/socket/SocketIO.h"

namespace netpp::internal::http::channel {
std::unique_ptr<internal::socket::ByteArray2IOVec> HttpChannelConversion::readBufferConvert(Channel *channel)
{
	auto httpChannel = dynamic_cast<HttpChannel *>(channel);
	return std::make_unique<internal::socket::ByteArrayReaderWithLock>(httpChannel->m_receiveArray);
}

std::unique_ptr<internal::socket::ByteArray2IOVec> HttpChannelConversion::writeBufferConvert(Channel *channel)
{
	auto httpChannel = dynamic_cast<HttpChannel *>(channel);
	return std::make_unique<internal::socket::SequentialByteArrayReaderWithLock>(
			std::forward<std::initializer_list<std::shared_ptr<ByteArray>>>(
					{httpChannel->m_sendHeaderArray, httpChannel->m_sendBodyArray}
			)
	);
}
}
