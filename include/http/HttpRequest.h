//
// Created by gaojian on 2021/12/18.
//

#ifndef NETPP_HTTPREQUEST_H
#define NETPP_HTTPREQUEST_H

#include "HttpCode.h"
#include <string>
#include <map>
#include <memory>

namespace netpp {
class ByteArray;
}

namespace netpp::http {
class HttpBaseRequest {
public:
	HttpBaseRequest();
	virtual ~HttpBaseRequest() = default;

	void setHttpVersion(const ProtocolVersion &version) { m_version = version; }
	/**
	 * @brief Get request http version
	 * @return http version
	 */
	[[nodiscard]] ProtocolVersion httpVersion() const { return m_version; }

	void setHeader(std::map<std::string, std::string> header);
	void setHeader(std::map<std::string, std::string> &&header);
	void addHeader(KnownHeader header, const std::string &value);
	void addHeader(KnownHeader header, std::string &&value);
	void addRawHeader(const std::string &header, const std::string &value);
	void addRawHeader(const std::string &header, std::string &&value);
	void addRawHeader(std::string &&header, const std::string &value);
	void addRawHeader(std::string &&header, std::string &&value);
	/**
	 * @brief Get request headers in key-value
	 * @return headers
	 */
	[[nodiscard]] std::map<std::string, std::string> headers() const { return m_header; }

	std::string getHeader(KnownHeader header) const;
	std::string getHeader(const std::string &header) const;
	std::string getHeader(std::string &&header) const;

	bool hasHeader(KnownHeader header) const;
	bool hasHeader(const std::string &header) const;
	bool hasHeader(std::string &&header) const;

	void setBody(std::shared_ptr<ByteArray> body);
	/**
	 * @brief Read http message body
	 * @return message body
	 */
	[[nodiscard]] std::shared_ptr<ByteArray> body();

private:
	ProtocolVersion m_version;
	std::map<std::string, std::string> m_header;
	std::shared_ptr<ByteArray> m_bodyBuffer;
};

class HttpRequest : public HttpBaseRequest {
public:
	HttpRequest();
	~HttpRequest() override = default;

	void setUrl(const std::string &url) { m_url = url; }
	void setUrl(std::string &&url) { m_url = std::move(url); }
	/**
	 * @brief Get request url
	 * @return url
	 */
	[[nodiscard]] std::string url() const { return m_url; }

	/**
	 * @brief Set request method
	 * @param method
	 */
	void setMethod(const RequestMethod &method) { m_method = method; }
	/**
	 * @brief Get http request method
	 * @return method
	 */
	[[nodiscard]] RequestMethod method() const { return m_method; }

private:
	RequestMethod m_method;
	std::string m_url;
};

class HttpResponse : public HttpBaseRequest {
public:
	HttpResponse();
	~HttpResponse() override = default;

	void setStatus(StatusCode code);
	[[nodiscard]] StatusCode status() const { return m_statusCode; }
	[[nodiscard]] std::string statusString() const { return m_status; }
private:
	StatusCode m_statusCode;
	std::string m_status;
};
}

#endif //NETPP_HTTPREQUEST_H
