//
// Created by gaojian on 2022/3/24.
//

#include "buffer/HttpChannel.h"
#include "buffer/ByteArray.h"
#include "buffer/ChannelBufferConversion.h"
#include "internal/http/HttpPackage.h"
#include "http/HttpRequest.h"

namespace netpp {
HttpChannel::HttpChannel(std::weak_ptr<SocketConnectionHandler> connection)
		: Channel(std::move(connection)),
		  m_httpParser{std::make_unique<HttpParser>()},
		  m_receiveArray{std::make_shared<ByteArray>()}
{}

HttpChannel::~HttpChannel() = default;

std::unique_ptr<internal::buffer::ChannelBufferConversion> HttpChannel::createBufferConvertor()
{
	return std::make_unique<netpp::internal::http::channel::HttpChannelConversion>();
}

std::unique_ptr<netpp::http::HttpRequest> HttpChannel::retrieveRequest()
{
	auto request = m_httpParser->decodeRequest(m_receiveArray);
	if (request)
		return std::make_unique<netpp::http::HttpRequest>(request.value());
	return nullptr;
}

std::unique_ptr<netpp::http::HttpResponse> HttpChannel::retrieveResponse()
{
	auto response = m_httpParser->decodeResponse(m_receiveArray);
	if (response)
		return std::make_unique<netpp::http::HttpResponse>(response.value());
	return nullptr;
}
}
