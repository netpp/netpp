//
// Created by gaojian on 2021/12/18.
//

#ifndef NETPP_HTTPREQUEST_H
#define NETPP_HTTPREQUEST_H

#include "HttpCode.h"
#include <string>
#include <map>

namespace netpp::http {
class HttpRequest {
public:
	HttpRequest();

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

	void setUrl(const std::string &url) { m_url = url; }
	void setUrl(std::string &&url) { m_url = std::move(url); }
	/**
	 * @brief Get request url
	 * @return url
	 */
	[[nodiscard]] std::string url() const { return m_url; }

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
	void addHeader(KnownHeader header, const char *value, std::size_t length);
	void addRawHeader(const std::string &header, const std::string &value);
	void addRawHeader(const std::string &header, std::string &&value);
	void addRawHeader(const std::string &header, const char *value, std::size_t length);
	void addRawHeader(std::string &&header, const std::string &value);
	void addRawHeader(std::string &&header, std::string &&value);
	void addRawHeader(std::string &&header, const char *value, std::size_t length);
	/**
	 * @brief Get request headers in key-value
	 * @return headers
	 */
	[[nodiscard]] std::map<std::string, std::string> headers() const { return m_header; }

	/**
	 * @brief Read http message body
	 * @return message body
	 */
	[[nodiscard]] std::string body() const;

private:
	RequestMethod m_method;
	std::string m_url;
	ProtocolVersion m_version;
	std::map<std::string, std::string> m_header;
};
}

#endif //NETPP_HTTPREQUEST_H
