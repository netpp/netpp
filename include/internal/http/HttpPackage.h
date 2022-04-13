#ifndef NETPP_HTTP_PACKAGE_H
#define NETPP_HTTP_PACKAGE_H

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
/**
 * @brief Parse incoming http message
 * 
 */
class HttpParser {
public:
	HttpParser();
	~HttpParser();
	std::optional<netpp::http::HttpRequest> decodeRequest(std::weak_ptr<ByteArray> byteArray);
	std::optional<netpp::http::HttpResponse> decodeResponse(std::weak_ptr<ByteArray> byteArray);

private:
	std::unique_ptr<DecoderImpl> m_impl;
};

/**
 * @brief Encode http request/response for sending
 * 
 */
class HttpPackage {
public:
	HttpPackage() = default;

	void encode(netpp::http::HttpRequest &request);
	void encode(netpp::http::HttpResponse &response);

private:
	std::shared_ptr<ByteArray> m_headerField;
	std::shared_ptr<ByteArray> m_bodyField;
};
}

#endif
