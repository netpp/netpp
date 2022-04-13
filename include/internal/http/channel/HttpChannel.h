//
// Created by gaojian on 2022/3/24.
//

#ifndef NETPP_HTTPCHANNEL_H
#define NETPP_HTTPCHANNEL_H

#include "channel/Channel.h"

namespace netpp {
class ByteArray;
namespace http {
class HttpRequest;
class HttpResponse;
}
namespace internal::http {
class HttpChannelConversion;
class HttpParser;
namespace channel {
/**
 * @brief A HttpChannel like a TcpChannel, provide buffer for TcpConnection,
 * however, the sending buffer can be mutable, depending on ready http request
 */
class HttpChannel : private Channel {
	friend class HttpChannelConversion;

public:
	using BufferConversion = HttpChannelConversion;

	explicit HttpChannel(std::weak_ptr<internal::handlers::TcpConnection> connection);
	~HttpChannel();

	std::unique_ptr<internal::buffer::ChannelBufferConversion> createBufferConvertor() override;

	/**
	 * @brief Try to get read a HttpRequest from channel
	 * @return nullptr if can not parse received data to request, otherwise a HttpRequest returned
	 */
	std::unique_ptr<netpp::http::HttpRequest> retrieveRequest();

	/**
	 * @brief Try to get read a HttpResponse from channel
	 * @return nullptr if can not parse received data to response, otherwise a HttpResponse returned
	 */
	std::unique_ptr<netpp::http::HttpResponse> retrieveResponse();

private:
	std::unique_ptr<HttpParser> m_httpParser;

	std::shared_ptr<ByteArray> m_receiveArray;

	std::shared_ptr<ByteArray> m_sendHeaderArray;
	std::shared_ptr<ByteArray> m_sendBodyArray;
};
}
}
}

#endif //NETPP_HTTPCHANNEL_H
