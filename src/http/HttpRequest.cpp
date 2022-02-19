//
// Created by gaojian on 2021/12/18.
//

#include "http/HttpRequest.h"

namespace netpp::http {
HttpRequest::HttpRequest()
	: m_method{RequestMethod::Get}, m_version{ProtocolVersion::Http2_0}
{}

void HttpRequest::setHeader(std::map<std::string, std::string> header)
{
	m_header = std::move(header);
}

void HttpRequest::setHeader(std::map<std::string, std::string> &&header)
{
	m_header = std::move(header);
}

void HttpRequest::addHeader(KnownHeader header, const std::string &value)
{
	const std::string_view &headerString = getHeader(header);
	addRawHeader(std::string(headerString), std::string(value));
}

void HttpRequest::addHeader(KnownHeader header, std::string &&value)
{
	const std::string_view &headerString = getHeader(header);
	addRawHeader(std::string(headerString), std::move(value));
}

void HttpRequest::addHeader(KnownHeader header, const char *value, std::size_t length)
{
	const std::string_view &headerString = getHeader(header);
	addRawHeader(std::string(headerString), std::string(value, length));
}

void HttpRequest::addRawHeader(const std::string &header, const std::string &value)
{
	addRawHeader(std::string(header), std::string(value));
}

void HttpRequest::addRawHeader(const std::string &header, std::string &&value)
{
	addRawHeader(std::string(header), std::move(value));
}

void HttpRequest::addRawHeader(const std::string &header, const char *value, std::size_t length)
{
	addRawHeader(std::string(header), std::string(value, length));
}

void HttpRequest::addRawHeader(std::string &&header, const std::string &value)
{
	addRawHeader(std::move(header), std::string(value));
}

void HttpRequest::addRawHeader(std::string &&header, std::string &&value)
{
	// add header implement here
	m_header.emplace(std::move(header), std::move(value));
}

void HttpRequest::addRawHeader(std::string &&header, const char *value, std::size_t length)
{
	addRawHeader(std::move(header), std::string(value, length));
}

std::string HttpRequest::body() const
{
	return std::string();
}
}
