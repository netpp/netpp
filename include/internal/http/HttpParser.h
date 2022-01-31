//
// Created by gaojian on 2021/12/19.
//

#ifndef NETPP_HTTPPARSER_H
#define NETPP_HTTPPARSER_H

#include <optional>
#include <memory>

namespace netpp {
class ByteArray;
namespace http {
class HttpRequest;
class HttpResponse;
}
}

namespace netpp::internal::http {
class DecoderImpl;
class HttpParser {
public:
	HttpParser();
	~HttpParser();
	std::optional<netpp::http::HttpRequest> decodeRequest(std::weak_ptr<ByteArray> byteArray);
	std::optional<netpp::http::HttpResponse> decodeResponse(std::weak_ptr<ByteArray> byteArray);
	void encode(std::weak_ptr<ByteArray> byteArray, const netpp::http::HttpRequest &request);
	void encode(std::weak_ptr<ByteArray> byteArray, const netpp::http::HttpResponse &response);

private:
	std::unique_ptr<DecoderImpl> m_impl;
};
}

#endif //NETPP_HTTPPARSER_H
